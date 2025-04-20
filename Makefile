# 编译器和通用选项
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
RM = rm -f

# 安装路径（可自定义）
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

# 程序目标
PROGS = background wm

# 链接参数（分程序配置）
LDFLAGS_background = -lxcb -lX11
LDFLAGS_wm = -lxcb -lxcb-util -lX11

# 默认编译所有程序
all: $(PROGS)

# 自动推导规则（隐式规则）
%: %.c
	$(CC) $< -o $@ $(CFLAGS) $(LDFLAGS_$@)

# 安装目标
install: all
	install -d $(DESTDIR)$(BINDIR)  # 确保目标目录存在
	install -m 755 $(PROGS) $(DESTDIR)$(BINDIR)

# 卸载目标
uninstall:
	$(RM) $(addprefix $(DESTDIR)$(BINDIR)/, $(PROGS))

# 清理目标
clean:
	$(RM) $(PROGS)