#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <stdlib.h>
#include <stdio.h>

#define BORDER_WIDTH 2
#define TITLEBAR_HEIGHT 20
#define BTN_SIZE 16
#define BG_COLOR 0x333333
#define BORDER_COLOR 0xfffff0
#define TITLE_COLOR 0xffffff
#define BTN_COLOR 0xff0000

typedef struct {
    xcb_window_t frame;
    xcb_window_t client;
    int x, y;
    uint16_t width, height;
    int is_moving;
    int is_resizing;
} Client;

xcb_connection_t *conn;
xcb_screen_t *screen;
xcb_window_t root;
Client *clients = NULL;

void draw_decoration(Client *c);

// 暴露事件处理函数
void handle_expose(xcb_expose_event_t *e) {
    for (int i = 0; clients[i].client; i++) {
        if (clients[i].frame == e->window) {
            draw_decoration(&clients[i]);
            break;
        }
    }
}

// 绘制窗口装饰
void draw_decoration(Client *c) {
    // 创建图形上下文
    uint32_t mask = XCB_GC_FOREGROUND;
    uint32_t value_list[1];
    xcb_gcontext_t gc = xcb_generate_id(conn);
    value_list[0] = BORDER_COLOR;
    xcb_create_gc(conn, gc, c->frame, mask, value_list);

    // 设置边框颜色
    xcb_change_window_attributes(conn, c->frame, XCB_CW_BORDER_PIXEL, value_list);

    // 绘制标题栏
    value_list[0] = TITLE_COLOR;
    xcb_change_gc(conn, gc, mask, value_list);
    xcb_rectangle_t rect = {0, 0, c->width, TITLEBAR_HEIGHT};
    xcb_poly_fill_rectangle(conn, c->frame, gc, 1, &rect);

    // 绘制关闭按钮
    value_list[0] = BTN_COLOR;
    xcb_change_gc(conn, gc, mask, value_list);
    xcb_rectangle_t btn = {
        c->width - BTN_SIZE - 2,
        2,
        BTN_SIZE,
        BTN_SIZE
    };
    xcb_poly_fill_rectangle(conn, c->frame, gc, 1, &btn);

    // 刷新显示
    xcb_flush(conn);

    // 释放GC
    xcb_free_gc(conn, gc);
}

// 处理鼠标按下事件
void handle_button_press(xcb_button_press_event_t *e) {
    Client *c = NULL;
    for (int i = 0; clients[i].client; i++) {
        if (clients[i].frame == e->event) {
            c = &clients[i];
            break;
        }
    }
    if (!c) return;

    // 检查关闭按钮点击
    if (e->event_x > c->width - BTN_SIZE - 2 && e->event_x < c->width - 2 &&
        e->event_y > 2 && e->event_y < 2 + BTN_SIZE) {
        xcb_destroy_window(conn, c->client);
        return;
    }

    // 检查标题栏拖动
    if (e->event_y < TITLEBAR_HEIGHT) {
        c->is_moving = 1;
        c->x = e->root_x - c->x;
        c->y = e->root_y - c->y;
    }
    
    // 检查边框调整大小
    if (e->event_x < BORDER_WIDTH || e->event_x > c->width - BORDER_WIDTH ||
        e->event_y < BORDER_WIDTH || e->event_y > c->height - BORDER_WIDTH) {
        c->is_resizing = 1;
    }
}

// 处理鼠标移动事件
void handle_motion_notify(xcb_motion_notify_event_t *e) {
    for (int i = 0; clients[i].client; i++) {
        Client *c = &clients[i];
        if (c->is_moving) {
            uint32_t values[] = {e->root_x - c->x, e->root_y - c->y};
            xcb_configure_window(conn, c->frame, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
        } else if (c->is_resizing) {
            uint16_t new_width = e->root_x - c->x;
            uint16_t new_height = e->root_y - c->y;
            uint32_t values[] = {new_width, new_height};
            xcb_configure_window(conn, c->frame, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
        }
    }
}

// 处理窗口创建
void handle_map_request(xcb_map_request_event_t *e) {
    Client c = {0};
    c.client = e->window;

    int i = 0;
    while (clients && clients[i].client) {
        i++;
    }

    // 在handle_map_request中修正clients扩展逻辑：
    clients = realloc(clients, (i+2)*sizeof(Client));
    clients[i] = c;
    clients[i+1] = (Client){0};  // 确保始终有终止符
    
    // 创建框架窗口
    c.frame = xcb_generate_id(conn);
    uint16_t client_width = screen->width_in_pixels / 2;  // 屏幕一半宽度
    uint16_t client_height = screen->height_in_pixels / 2; // 屏幕一半高度
    xcb_create_window(
        conn,
        screen->root_depth,
        c.frame,
        root,
        0, 0,
        client_width, client_height,
        BORDER_WIDTH,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
        (const uint32_t[]){
            BG_COLOR,
            XCB_EVENT_MASK_EXPOSURE |
            XCB_EVENT_MASK_BUTTON_PRESS |
            XCB_EVENT_MASK_BUTTON_RELEASE |
            XCB_EVENT_MASK_POINTER_MOTION
        }
    );

    // 重新设置父窗口位置（Y坐标为标题栏高度）
    xcb_reparent_window(conn, c.client, c.frame, 0, TITLEBAR_HEIGHT);

    // 绘制装饰
    draw_decoration(&c);
    xcb_map_window(conn, c.frame);

    // 保存客户端
    i = 0;
    while (clients && clients[i].client) {
        i++;
    }
    clients = realloc(clients, (i+2)*sizeof(Client));
    clients[i] = c;
    clients[i+1] = (Client){0};
}

int main() {
    clients = malloc(2 * sizeof(Client));  // 预分配空间
    clients[0] = (Client){0};
    conn = xcb_connect(NULL, NULL);
    screen = xcb_aux_get_screen(conn, 0);
    root = screen->root;

    // 设置根窗口背景
    xcb_change_window_attributes(conn, root, XCB_CW_BACK_PIXEL, (uint32_t[]){BG_COLOR});
    xcb_clear_area(conn, 0, root, 0, 0, 0, 0);

    // 事件监听（添加暴露事件处理）
    xcb_generic_event_t *event;
    while ((event = xcb_wait_for_event(conn))) {
        switch (event->response_type & ~0x80) {
            case XCB_MAP_REQUEST:
                handle_map_request((xcb_map_request_event_t*)event);
                break;
            case XCB_BUTTON_PRESS:
                handle_button_press((xcb_button_press_event_t*)event);
                break;
            case XCB_MOTION_NOTIFY:
                handle_motion_notify((xcb_motion_notify_event_t*)event);
                break;
            case XCB_EXPOSE:
                handle_expose((xcb_expose_event_t*)event);
                break;
        }
        free(event);
    }

    xcb_disconnect(conn);
    return 0;
}