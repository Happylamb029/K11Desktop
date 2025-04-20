# K11Desktop
一个很正常的X11窗口管理器，现在把背景做完了！
好吧，我不知道成品是咋样的，因为我的Xephyr他用不了=(

# 如何编译！

---

#### **1. 安装依赖库**
根据 Linux 发行版选择对应的安装命令：

| **发行版**       | **安装命令**                                                                 |
|------------------|-----------------------------------------------------------------------------|
| **Ubuntu/Debian** | ```bash<br>sudo apt-get install libxcb-dev libxcb-util-dev libx11-dev<br>``` |
| **Arch/Manjaro**  | ```bash<br>sudo pacman -S xcb-util xcb libxcb xorg-x11-devel<br>```           |
| **Fedora**       | ```bash<br>sudo dnf install libxcb-devel xcb-util-devel libX11-devel<br>```  |
| **openSUSE**     | ```bash<br>sudo zypper install libxcb-devel xcb-util-devel libX11-devel<br>```|

---

#### **2. 编译命令**
分别编译两个程序：

##### **编译 `background.c`**
```bash
gcc background.c -o background -lxcb -lX11
```

##### **编译 `wm.c`**
```bash
gcc wm.c -o wm -lxcb -lxcb-util -lX11
```

---

#### **3. 参数说明**
- `-lxcb`：链接 XCB 核心库
- `-lxcb-util`：链接 XCB 辅助工具库（用于 `xcb_aux.h`）
- `-lX11`：链接 X11 基础库（部分系统需要）

---

#### **4. 运行程序**
```bash
# 运行背景程序（需后台运行）
./background &

# 运行窗口管理器（需 X11 环境）
./wm
```

---

### **注意事项**
#### **常见问题**
1. **`xcb_aux.h not found`**  
   - 确保安装了 `libxcb-util-dev`（Ubuntu）或 `xcb-util`（Arch）。
   - 编译时使用 `-lxcb-util` 而非 `-lxcb_aux`。

2. **`undefined reference to 'xcb_aux...'`**  
   - 确保链接参数包含 `-lxcb-util`。

3. **`error: ‘xcb_aux_get_screen’ was not declared`**  
   - 需要更新 XCB 库到较新版本（如 Ubuntu 20.04 可能需要手动编译 XCB）。

---

#### **依赖库验证**
检查库是否安装成功：
```bash
# 查找头文件
find /usr -name "xcb_aux.h"

# 查找动态库
ldconfig -p | grep xcb-util
```

---

#### **高级配置**
- **动态库路径问题**：若提示缺少 `.so` 文件，可添加路径：
  ```bash
  export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH
  ```
- **调试编译**：添加 `-g` 参数生成调试符号：
  ```bash
  gcc -g wm.c -o wm -lxcb -lxcb-util -lX11
  ```
---

### **环境要求**
- 需在 X11 环境下运行（不支持 Wayland）
- 确保显示器配置正确（`DISPLAY=:0`）

这样就OK了！运行一下吧！
