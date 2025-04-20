#include <xcb/xcb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

xcb_connection_t *conn;
xcb_screen_t *screen;
xcb_window_t bg_window;

void load_background(uint32_t color) {
    // 创建背景窗口
    bg_window = xcb_generate_id(conn);
    xcb_create_window(
        conn, XCB_COPY_FROM_PARENT, bg_window, screen->root,
        0, 0, screen->width_in_pixels, screen->height_in_pixels, 0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
        (uint32_t[]){ 
            color, 
            XCB_EVENT_MASK_EXPOSURE 
        }
    );

    // 设置窗口为最底层
    xcb_configure_window(
        conn, bg_window,
        XCB_CONFIG_WINDOW_STACK_MODE,
        (uint32_t[]){ XCB_STACK_MODE_BELOW }
    );

    xcb_map_window(conn, bg_window);
    xcb_flush(conn);
}

int main() {
    conn = xcb_connect(NULL, NULL);
    screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
    
    // 设置纯色背景
    uint32_t color = 0x20B2AA;
    load_background(color);
    
    // 保持连接打开
    while (1) {
        xcb_flush(conn);
        sleep(1);
    }
    
    return 0;
}