# liblcd GUI 底层框架分析

## 1. 概述

`liblcd` 是一个面向嵌入式设备（电表/采集终端）的单色 LCD 图形用户界面底层库。屏幕分辨率为 **160×160 像素**，每个像素用 1 bit 表示（单色，黑白）。框架采用 C++ 编写，使用经典的 GUI 架构模式：**视图组件树 + 窗体导航栈 + 事件驱动键盘输入**。

---

## 2. 整体架构分层

```
┌──────────────────────────────────────────────────────────┐
│                    应用层 (Menu/Config)                    │
├──────────────────────────────────────────────────────────┤
│  Form 层 (Screen, EditForm, MenuForm, ListForm, ...)      │
├──────────────────────────────────────────────────────────┤
│  Component 层 (ViewComponent, Label, Button, Icon)        │
├──────────────────────────────────────────────────────────┤
│  渲染层 (Font + MatrixBit + Canvas)                       │
├──────────────────────────────────────────────────────────┤
│  硬件抽象层 (graphics → /dev/fb0 或 /dev/fb/0)            │
├──────────────────────────────────────────────────────────┤
│  输入层 (keyBoard → /dev/gpio_kbd 或 /dev/input/event2)   │
└──────────────────────────────────────────────────────────┘
```

---

## 3. 基础类型定义

### 3.1 坐标与位置 (`types.hpp`)

```cpp
struct Point { int x; int y; };       // 像素坐标
struct Position { int col; int row; }; // 行列坐标
```

### 3.2 全局常量 (`typedef.hpp`)

- `MAXROW = 160` — 屏幕最大行数（Y方向像素）
- `MAXCOL = 80` — 屏幕最大列数（X方向字节数，每字节8像素，共160像素）
- 按键编码: `K1_UP=1, K2_ESC=2, K3_ENTER=4, k4_DOWN=8`

### 3.3 公共函数 (`public.hpp/cpp`)

```cpp
int min(int x, int y);  // 返回较小值
```

---

## 4. 硬件抽象层 (HAL)

### 4.1 图形设备抽象 — `graphics` 类 (`graphics.hpp/cpp`)

```
                     graphics (抽象基类)
                    /                  \
           COG_graphics           TAB_graphics
        (COG Chip On Glass)    (TAB LCD, mmap方式)
```

**关键常量：**
- `actX = 160, actY = 160` — 实际屏幕尺寸
- `DISP_X_SIZE = 160, DISP_Y_SIZE = 160`
- `BITS_PER_PIXER = 1` — 每像素1bit（单色）
- `DISP_BUFF_SIZE = 160 * 160 * 1 / 8 = 3200 字节`
- `BYTES_PER_LINE = 160 / 8 = 20 字节`

**`graphics` 抽象接口：**
```cpp
class graphics {
public:
    virtual void sync(char *buf, int bufsize) = 0;  // 将缓冲区数据同步到显示设备
protected:
    virtual bool openDev(const char *pDevName) = 0;
    virtual bool closeDev() = 0;
};
```

**`TAB_graphics`** — 通过 `mmap` 内存映射方式直接操作 `/dev/fb/0` 帧缓冲设备。将文件映射到内存后直接在映射区读写，效率高。

**`COG_graphics`** — 通过 `write()` 系统调用向 `/dev/fb0` 写入显存数据。在 `USE_SIMULATOR` 宏定义下，使用 **SDL** 库在 PC 上模拟 LCD 显示（320×240 窗口，2倍放大）。

**设备初始化 (`InitGUIDevice`)：** 自动检测 `/dev/fb0`（COG）或 `/dev/fb/0`（TAB）设备文件是否存在，选择合适的驱动。

---

### 4.2 键盘输入 — `keyBoard` 类 (`keyboardManage.hpp/cpp`)

```
keyBoard
├── 支持 FOUR_KEYS (4键) 和 SIX_KEYS (6键) 两种键盘布局
├── 键值枚举: MYKEY_UP/DOWN/LEFT/RIGHT/ENTER/CANCEL/NULL
├── 读取设备: /dev/gpio_kbd 或 /dev/input/event2 (全志平台)
├── 背光控制: 通过 /proc/osal/lcd_backlight 或键盘设备写0xFF/0x00
└── 模拟器: 使用 SDL 键盘事件 (方向键+回车+ESC)
```

**按键映射表：**
- 4键模式: `{NULL, ENTER, DOWN, NULL, CANCEL, NULL, NULL, NULL, UP}`
- 6键模式: `{NULL, ENTER, DOWN, NULL, CANCEL, LEFT, NULL, RIGHT, UP}`

背光有自动关闭机制：60秒无按键操作自动熄灭背光以省电。

---

## 5. 核心渲染层

### 5.1 画布 — `Canvas` 类 (`canvas.hpp/cpp`)

`Canvas` 是整个图形系统的核心，管理显存缓冲区和所有像素级绘制操作。

#### 5.1.1 双缓冲区设计

```
zone[384][20]     ← 虚拟画布 (384×384 像素, 384*20=7680 bytes)
                        ↓ Sync()
fb_buffer[160][20] ← 物理帧缓冲 (160×160 像素, 160*20=3200 bytes)
                        ↓ Sync2Hw()
graphics::sync()   → 硬件LCD屏幕
```

- **`zone`**: 虚拟画布，384×384 像素，支持大于物理屏幕的绘制区域，实现滚动/翻页
- **`fb_buffer`**: 物理帧缓冲区，160×160 像素
- **`Sync()`**: 将 zone 中当前窗口区域复制到 fb_buffer
- **`Sync2Hw()`**: 将 fb_buffer 通过 graphics 驱动同步到硬件

所有绘制操作直接写入 `zone` 数组。每个字节对应 8 个像素（水平方向），`zone[y][x>>3]` 定位到像素所在字节，`value = 0x01 << (7 - (x & 0x07))` 计算该像素在字节中的位。

#### 5.1.2 窗口系统（虚拟滚动）

```cpp
Canvas::SetSize(left, right, top, bottom);    // 设置逻辑画布区域
Canvas::SetWindowSize(xlen, ylen);             // 设置可视窗口大小
Canvas::SetWindowOrig(x, y);                   // 设置可视窗口在逻辑画布上的起点
Canvas::isPointInWindow(pnt);                  // 判断点是否在当前窗口内
```

例如菜单过长时，通过 `SetWindowOrig(0, y)` 实现垂直滚动，每次只绘制可视窗口内的内容到物理屏幕。

#### 5.1.3 像素级绘制 — `DrawBit`

这是所有图形操作的最底层原语：

```cpp
bool Canvas::DrawBit(int x, int y, bool isBlack) {
    unsigned char value = 0x01 << (7 - (x & 0x07));
    if (isBlack)
        zone[y][x >> 3] |= value;   // 置位 = 黑色
    else
        zone[y][x >> 3] &= ~value;  // 清零 = 白色
}
```

#### 5.1.4 画线实现 — `DrawLine`

```
DrawLine(x1,y1, x2,y2)
    ├─ 两点重合 → DrawBit (画点)
    ├─ y1==y2 → DrawHorizonLine (水平线，优化)
    ├─ x1==x2 → DrawVerticalLine (垂直线，优化)
    └─ 其他 → DrawSlashLine (斜线，Bresenham算法)
```

**水平线优化 (`DrawHorizonLine`)：**
- 计算起点字节索引和终点字节索引
- 中间的完整字节用 `memset` 一次性填充 `0xFF`（黑）或 `0x00`（白）
- 首尾非完整字节用位运算处理，大幅提升性能

```cpp
// 中间完整字节直接赋值
if (middle_len > 0) {
    if (isBlack)
        memset(&zone[y][start_cpy_pos], 0xff, middle_len);
    else
        memset(&zone[y][start_cpy_pos], 0, middle_len);
}
```

**垂直线 (`DrawVerticalLine`)：** 逐行对同一列位置置位/清零。

**斜线 (`DrawSlashLine`)：** 使用 **Bresenham 直线算法**，分 8 个象限（octant）处理：
- dx≥0, dy≥0, dx≥dy → 第1象限
- dx≥0, dy≥0, dx<dy → 第2象限
- dx<0, dy≥0, ... → 第3/4象限
- dx<0, dy<0, ... → 第5/6象限
- dx≥0, dy<0, ... → 第7/8象限

```cpp
// Bresenham 核心逻辑（第1象限）:
e = dy - dx / 2;
while (x1 <= x2) {
    DrawBit(x1, y1, isBlack);
    if (e > 0) { y1 += 1; e -= dx; }
    x1 += 1;
    e += dy;
}
```

#### 5.1.5 矩形 — `DrawRectangle`

用四条 `DrawLine` 拼成矩形边框：
```cpp
DrawLine(x1,y1, x2,y1, true);  // 上边
DrawLine(x2,y1, x2,y2, true);  // 右边
DrawLine(x1,y2, x2,y2, true);  // 下边
DrawLine(x1,y1, x1,y2, true);  // 左边
```

#### 5.1.6 反色/高亮 — `Reverse`

```cpp
// 全区域反色
zone[y][x] ^= 0xff;

// 矩形区域反色（用于高亮选中项）
Canvas::Reverse(lefttop, rightbtm);
```

#### 5.1.7 线程安全

使用 POSIX 递归互斥锁 `pthread_mutex_t` 保护 `zone` 缓冲区：
```cpp
static void lock();    // pthread_mutex_lock(&zone_mut)
static void unlock();  // pthread_mutex_unlock(&zone_mut)
```

---

### 5.2 位图矩阵 — `MatrixBit` 类 (`matrixbit.hpp/cpp`)

`MatrixBit` 负责从**位图字库文件**中读取字符/图标的点阵数据，并逐点绘制到 Canvas。

```
MatrixBit
├── SetXYBits(X, Y)  — 设置位图宽高，分配内存 matrix[Xchars*Y]
├── SetFile(filename) — 设置字库文件路径
├── SetOffset(offset) — 设置文件偏移（字符在字库中的位置 = offset * ichars）
├── GetMatrix()       — 从文件 fseek 读取 ichars 字节到 matrix 缓冲区
└── Draw(x, y, isBlack) — 逐像素绘制到 Canvas
```

**字库文件格式：** 二进制点阵文件，每个字符/图标连续存储 `(X+7)/8 * Y` 字节。字符在字库中的偏移量 = 字符序号 × 每个字符的字节数。

**Draw 实现 — 逐位展开：**
```cpp
bool MatrixBit::Draw(int x, int y, bool isBlack) {
    for (int i = 0; i < iYbits; i++) {
        for (int j = 0; j < iXchars; j++) {
            int _x = x + j * 8 + 7;
            unsigned char tmp = matrix[i * iXchars + j];
            for (int k = 0; k < 8; k++) {
                if (tmp & 0x01)
                    canvas->DrawBit(_x, y, !isBlack); // 1=亮=背景色
                else
                    canvas->DrawBit(_x, y, isBlack);  // 0=暗=前景色
                tmp >>= 1;
                _x--;
            }
        }
        y++;
    }
}
```

数据从字节 LSB 开始输出，对应像素从左到右（大端位序），`_x--` 每次递减，所以最右边的像素对应字节的最低位。

---

### 5.3 字体系统 — `Font` 类 (`font.hpp/cpp`)

```
Font (每种字号一个全局实例)
├── Font16 (16×16汉字, 8×16 ASCII)
├── Font12 (12×12汉字, 8×12 ASCII)
└── Font8  (无汉字, 8×8 ASCII)
```

**字体属性表：**
| 类型 | 高度 | 汉字宽度 | ASCII宽度 | 汉字字库 | ASCII字库 |
|------|------|----------|-----------|----------|-----------|
| FONT16 | 16 | 16 | 8 | /usr/fonts/myhzk1616 | /usr/fonts/myascii168 |
| FONT12 | 12 | 12 | 8 | /usr/fonts/myhzk1212 | /usr/fonts/myascii128 |
| FONT8 | 8 | 0 | 8 | /usr/fonts/myhzk1212 | /usr/fonts/myascii88 |

**内部结构：** 每个 Font 包含两个 `MatrixBit` 对象：
- `asciibit` — 用于 ASCII 字符
- `hzbit` — 用于汉字字符

**文本输出 (`TextOut`)：** 逐字符判断是否 ASCII（字节值 < 128）：
- **ASCII 字符**：字节值直接作为字库偏移，用 `asciibit` 绘制
- **汉字（GB2312）**：区位码计算 — `offset = (byte0 - 1 - 0xA0) × 94 + (byte1 - 1 - 0xA0)`，用 `hzbit` 绘制

```cpp
while (*ptext) {
    if ((unsigned char)ptext[0] < 128) {
        // ASCII: 直接以字节值为偏移
        asciibit.SetOffset((unsigned char)ptext[0]);
        asciibit.Draw(x, y, isBlack);
        x += ascii_x;
    } else {
        // 汉字: GB2312 区位码计算
        offset = (ptext[0] - 1 - 0xA0) * 94 + ptext[1] - 1 - 0xA0;
        hzbit.SetOffset(offset);
        hzbit.Draw(x, y, isBlack);
        x += hz_x;
        ptext++;  // 汉字占两个字节
    }
    ptext++;
}
```

---

## 6. 组件层 (Component Layer)

### 6.1 继承层次

```
ViewComponent (抽象基类)
├── Label       — 文本标签
│   └── Button  — 按钮 (带点击事件)
│       └── ParamButton — 带参数的命令按钮
├── Icon        — 图标
└── Form        — 窗体(容器)
    ├── Screen  — 屏幕级窗体
    │   ├── EditForm     — 编辑窗体
    │   ├── MaskEditForm — 掩码编辑窗体
    │   ├── ASCIIEditForm— ASCII软键盘编辑
    │   ├── PassChkForm  — 密码验证
    │   ├── PasswordSetForm — 密码设置
    │   ├── MenuForm     — 菜单窗体
    │   └── MessageForm  — 消息提示窗体
    ├── ListForm — 列表窗体
    │   └── CompListForm — 复合列表(多列多页)
    ├── TopState    — 顶部状态栏
    └── BottomState — 底部状态栏
```

### 6.2 `ViewComponent` — 所有可显示组件的根 (`viewcomponent.hpp/cpp`)

```cpp
class ViewComponent {
    Point pst;          // 组件位置 (x,y)
    bool isenabled;     // 是否启用/聚焦
    bool isvisible;     // 是否可见
    Canvas *canv;       // 关联的画布

    virtual void show();      // 绘制自身
    virtual void enable();    // 获得焦点
    virtual void disable();   // 失去焦点
    virtual void click();     // 点击/确认
    virtual void setfocus(bool state);
};
```

**设计特点：**
- 位置由 `Point` 坐标（像素）表示
- 每个组件持有画布指针，通过画布进行绘制
- `enable/disable` 控制焦点状态
- `click()` 为虚函数，子类重写实现具体点击行为

### 6.3 `Label` — 文本标签 (`label.hpp/cpp`)

```cpp
class Label : public ViewComponent {
    Point startpst, endpst;  // 文本区域的起止坐标
    char *text;              // 文本内容（动态分配）
    Font *font;              // 使用的字体（默认 Font16）
    bool focused;            // 是否聚焦（反色显示）

    void show();             // 在画布上绘制文本
    void settext(const char*);
    void setfmttext(const char *fmt, ...);  // 格式化文本
    void setvalue(int/long/float/double);    // 数值转文本
    void left()/right()/center();           // 文本对齐
};
```

**`show()` 实现：** 调用 `font->TextOut(startpst.x, startpst.y, text, focused)`，当 `focused=true` 时文本反色显示（黑底白字）。

### 6.4 `Button` — 按钮 (`button.hpp/cpp`)

```cpp
class Button : public Label {
    ViewComponent *_sender;  // 点击后通知的目标组件

    void click() {
        // 如果_sender是EditForm，将按钮文本设置到编辑框
        // 如果_sender是MaskEditForm，将按钮文本设置到掩码编辑框
        // 然后enable _sender
    }
};
```

**`ParamButton`** — 扩展 Button，支持命令行参数和自定义回调函数：
```cpp
class ParamButton : public Button {
    int argc;
    char *argv[11];
    CommandFunc _func;  // void (*)(int argc, char *argv[])

    void click() {
        if (_func) _func(argc, argv);  // 执行回调
        else _sender->enable();         // 或激活目标组件
    }
};
```

### 6.5 `Icon` — 图标 (`icon.hpp/cpp`)

```cpp
class Icon : public ViewComponent {
    MatrixBit matrix;       // 内部用 MatrixBit 加载图标数据
    char *filename;         // 图标文件路径
    int pictures;           // 文件中图片总数

    bool SetPic(int sn);    // 选择第sn个图片
    void show() {
        matrix.Draw(_x, _y, false);  // 在指定位置绘制图标
    }
};
```

图标文件是一个包含多个等大图片的二进制点阵文件，通过 `SetPic(n)` 切换显示不同图片（如信号强度图标的不同格数）。

---

## 7. 窗体层 (Form Layer)

### 7.1 `Form` — 窗体和导航系统 (`form.hpp/cpp`)

```cpp
class Form : public ViewComponent {
    Form *parent;                           // 父窗体指针
    vector<ViewComponent*> components;      // 子组件列表（全部）
    vector<ViewComponent*> tabcomponents;   // 可被Tab切换的子组件（可聚焦的）
    unsigned int taborder;                  // 当前获得焦点的组件索引
    Canvas canvas;                          // 每个Form有自己的Canvas
    static Form *current;                   // 当前活动窗体（全局单例）
    char hint[MAX_HINT_LEN];               // 底部提示文字

    void addcomponent(ViewComponent*, bool tab);  // 添加子组件
    void enable();   // 激活此窗体（设为current，重绘）
    void disable();  // 返回父窗体
    void key_enter();     // 确认：激活焦点组件的click()
    void key_up/down();   // 上下键：切换焦点组件
    void key_cancel();    // 取消：返回父窗体
};
```

**导航机制：**
- `Form::current` 静态指针始终指向当前活动窗体
- `enable()` 将自身设为 `current` 并绘制
- `disable()` 将 `current` 恢复为 `parent` 并激活父窗体
- 所有键盘事件通过 `Form::keydown()` 分发到 `current->key_xxx()`

**Tab导航：**
- 上下键在 `tabcomponents` 列表中循环切换
- `settaborder(n)` 设置第 n 个 tab 组件获得焦点
- 焦点切换时先 `disable` 当前组件，再 `enable` 下一个

**`Form::show()` 流程：**
```
lock Canvas
    canvas.clear()           // 清空画布
    for each component:
        if visible:
            component.show() // 各组件自行绘制
    canvas.Sync()            // 同步到物理缓冲
unlock Canvas
```

### 7.2 `Screen` — 屏幕级窗体 (`screen.hpp/cpp`)

```cpp
class Screen : public Form {
    bool isSetParam;   // 是否是参数设置画面
    bool isMultiShow;  // 是否多页显示

    void enable();     // 激活前先检查安全等级，必要时弹出密码验证
    void disable();    // 恢复父窗体的底部提示
    void JudgeAndSetWindow(); // 自动滚动窗口使焦点组件可见
};
```

**自动滚动机制 (`JudgeAndSetWindow`)：** 当焦点组件不在当前可视窗口内时，自动切换虚拟窗口到上半部分（y=0~130）或下半部分（y=130~260），实现类似翻页的效果。

**安全机制：** `enable()` 时检查 `securelevel`，非 `LOW` 安全等级需要先通过密码验证。

---

## 8. 专用窗体详解

### 8.1 `EditForm` — 字符编辑窗体 (`editform.hpp/cpp`)

```
继承: Screen
功能: 逐位编辑字符串，支持多种字符集
布局: 标题 + 编辑内容 + 提示文字 + [OK]按钮
```

**编辑类型 (`EDITTYPE`)：**
| 类型 | 字符表 | 用途 |
|------|--------|------|
| EDT_DIGIT | 0-9 | 数字编辑 |
| EDT_ALPHA | a-z | 字母编辑 |
| EDT_NETADDR | a-z + 0-9 + . | 网络地址 |
| EDT_METERADDR | 0-9 + a | 表地址 |
| EDT_HEXDIGIT | 0-9 + A-F | 十六进制 |

**交互逻辑：**
- 左右键移动编辑光标位置
- 上下键在当前字符表中循环切换字符
- 光标定位到最后一个"OK"位置时按确认键提交
- 当前编辑位通过 `canvas.Reverse()` 反色高亮显示

**反色光标实现 (`show()`)：**
```cpp
// 绘制编辑框矩形
canvas.DrawRectangle(pst.x-1, pst.y-1, pst.x+wid, pst.y+18, true);
// 反色高亮当前编辑位
canvas.Reverse(start, end);  // 反色一个 8×16 像素区域
```

### 8.2 `MaskEditForm` — 掩码编辑窗体 (`maskeditform.hpp/cpp`)

```
继承: Screen
功能: 按掩码格式编辑，如 "##.##.##" 形式的IP地址
```

**掩码规则：** `mask` 字符串中 `#` 表示可编辑位，其他字符为固定分隔符（如 `.` `-` `:`）。

**去除前导零 (`STRIP_TYPE`)：**
- `NO_STRIP` — 保留所有字符
- `HEAD_ZERO` — 去除头部无效零（用于带小数点的数值）
- `ALL_INVALID_ZERO` — 去除所有段的前导零

### 8.3 `ASCIIEditForm` — ASCII软键盘 (`asciieditform.hpp/cpp`)

```
继承: Screen
功能: 5行×10列矩阵键盘，可选择输入字母/数字/符号
布局:
  [a][b][c]...[j]       (第1行)
  [k][l][m]...[t]       (第2行)
  [u][v][w]...[z]       (第3行)
  [0][1][2]...[9]       (第4行)
  [.][_][:][+][-][@][!][*][#][%]  (第5行前8个)
  [CapsLock] [Clear] [OK]   (特殊按钮)
```

- 含 CapsLock 大小写切换（对字母行 ±32）
- 含 Clear 清空功能
- 输入结果显示在顶部一行，最多16字符

### 8.4 `ListForm` — 列表窗体 (`listform.hpp/cpp`)

```
继承: Form
功能: 垂直列表选择
内部: vector<Label*> 存储列表项
```

- `additem()` 动态创建 Label 加入列表
- 上下键移动选择
- 选中项通过 `Form::key_up/down` 的反色机制高亮

### 8.5 `CompListForm` — 复合列表 (`complistform.hpp/cpp`)

```
继承: ListForm
功能: 多列多页列表，如矩阵选择界面
```

- 支持多列布局（`maxcolums × maxrows` 网格）
- 支持多页翻页
- 左右键切换列，上下键切换行
- 到边界时自动翻页

### 8.6 `MenuForm` — 菜单系统 (`menuform.hpp/cpp`)

```
继承: Screen
功能: 多级菜单，支持XML定义
内部: 链表 MenuItemList 管理菜单项
```

**菜单数据结构：**
```cpp
struct MenuItemList {
    ParamButton *btn;
    MenuItemList *next;
};
```

**XML菜单加载：** `LoadXmlFile()` 使用 TinyXML 解析菜单配置文件，递归构建菜单树。每个 `<mainmenu>` 和 `<submenu>` 节点创建对应的 `MenuForm`。

**菜单显示/隐藏 (`ShowMenu`)：** 支持动态隐藏/显示部分菜单项（用于不同权限下的菜单切换）。

### 8.7 `MessageForm` — 消息提示框 (`messageform.hpp/cpp`)

```
继承: Screen
功能: 显示多行消息文本，支持翻页
布局: 标题 + 6行文本 (每行20个ASCII字符)
```

- `MAX_CHARS_PAGE = 120` (20×6) 字符每页
- 上下键翻页
- 自动处理中英文混合断行（不在汉字中间截断）

### 8.8 密码系统 (`passwdform.hpp/cpp`, `passwddoc.hpp/cpp`)

```
PassChkForm (密码验证)
├── 6位数字密码
├── 5次错误锁定24小时
├── 超级密码 "211314" 始终有效
└── PassChkForm_Ascii (ASCII密码版本, 支持8位)

PasswordSetForm (密码设置)
├── 两次输入确认
└── PasswordSetForm_AscII (ASCII版本)
```

**`PasswdDoc`** — 密码持久化：读/写 `/home/app/passwd` 文件（7字节二进制）。

---

## 9. 状态栏

### 9.1 `TopState` — 顶部状态栏 (`topstate.hpp/cpp`)

```
继承: Form
区域: 0~160×0~17 (顶部17像素)
显示内容:
  [在线图标] [信号图标] [4G/3G/2G] [!告警] [测量点ID] [时间] [电池]
```

**定时刷新机制：** `SyncTimeAndDealBlink()` 每秒钟更新一次时间，同时处理：
- 信号强度图标切换（0~4格）
- 告警图标闪烁（1Hz）
- 在线图标闪烁（1Hz）
- 通信类型显示（4G/3G/2G/L/Ethernet）
- 电池告警指示

### 9.2 `BottomState` — 底部状态栏 (`bottomstate.hpp/cpp`)

```
继承: Form
区域: 0~160×147~160 (底部13像素)
显示内容:
  [提示文字] + [方向箭头图标]
```

**箭头类型：** `EMPTY`(无), `TO_PREV`(上箭头), `TO_NEXT`(下箭头), `PREV_NEXT`(双向箭头)

箭头用于指示用户当前页面还有更多内容可翻页查看。

---

## 10. 消息主循环 (`msginfo.cpp`)

程序入口 `main()` 的简化流程：

```
1. 检查 /dev/ST7567LCD 设备是否存在
2. 初始化键盘 (new keyBoard)
3. 初始化图形设备 (Canvas::Init)
4. 如果命令行有参数，显示为消息
5. 否则等待消息循环（外部调用 Form::keydown）
```

---

## 11. 一个像素如何到达屏幕 — 完整数据流

以绘制一个汉字 "测" 为例：

```
1. Font::TextOut(x, y, "测", true)
   │
2. 判断字节 ≥ 128，识别为汉字
   │  计算偏移 = (0xB2-1-0xA0)*94 + (0xE2-1-0xA0) = 特定序号
   │
3. hzbit.SetOffset(offset)  → 设置字库文件偏移
   │
4. hzbit.Draw(x, y, true)
   │
5. MatrixBit::GetMatrix()
   │  fopen("/usr/fonts/myhzk1616")
   │  fseek 到 offset*32 (16×16/8*16=32字节)
   │  fread 32 字节到 matrix 缓冲区
   │
6. MatrixBit::Draw() 逐字节展开
   │  for i in 0..15:      // 16行
   │    for j in 0..1:     // 2字节/行
   │      for k in 0..7:   // 每字节8位
   │        canvas->DrawBit(_x, y, isBlack)
   │
7. Canvas::DrawBit(x, y, true)
   │  value = 0x01 << (7 - (x & 0x07))
   │  zone[y][x >> 3] |= value   ← 写入虚拟画布
   │
8. Canvas::Sync()
   │  memcpy(&fb_buffer[y2][x2], &zone[y1][x1], x_chars*yLen)
   │                               ← 拷贝窗口区域到物理缓冲
   │
9. Canvas::Sync2Hw()
   │  device->sync(fb_buffer, 3200)
   │
10. COG_graphics::sync()
    │  write(fd, buf, 3200)     ← 写入 /dev/fb0
    │  或 (模拟器)
    │  SDL_BlitSurface + SDL_UpdateRects  ← 显示在SDL窗口
    │
11. LCD屏幕显示汉字 "测" ✓
```

---

## 12. 类关系总图

```
                    ┌─────────────┐
                    │  keyBoard   │ (键盘输入)
                    └─────────────┘
                           │
                    ┌──────▼──────┐
                    │  graphics   │ (LCD硬件驱动)
                    │  △     △   │
                    │ COG   TAB   │
                    └──────┬──────┘
                           │
              ┌────────────▼────────────┐
              │        Canvas           │
              │  zone[384][20]          │ (虚拟画布)
              │  fb_buffer[160][20]     │ (物理缓冲)
              │  DrawBit/DrawLine/...   │ (像素绘制)
              │  lock/unlock/Sync       │ (同步控制)
              └──────▲─────▲───────────┘
                     │     │
         ┌───────────┘     └──────────┐
         │                            │
   ┌─────┴─────┐              ┌──────┴──────┐
   │ MatrixBit │              │ ViewComponent│
   │ 位图矩阵  │              │  △          │
   └─────┬─────┘              │  ├──Label   │
         │                    │  │  └─Button│
   ┌─────┴─────┐              │  ├──Icon    │
   │   Font    │◄─────────────┤  └──Form    │
   │  (用两个   │  引用        │      △     │
   │  MatrixBit)│              │      ├Screen
   └───────────┘              │      ├ListForm
                              │      │  └CompListForm
                              │      ├TopState
                              │      ├BottomState
                              │      └MessageForm
                              └──────────────┘
```

---

## 13. 关键设计模式与特点

1. **虚拟画布 + 物理缓冲 + 硬件同步** — 三级缓冲架构，支持逻辑画布大于物理屏幕，实现高效滚动。

2. **窗体栈导航** — `Form::current` + `parent` 链构成窗体导航栈，`enable/disable` 实现窗体的压栈/出栈。

3. **Tab焦点管理** — 每个 Form 维护 `tabcomponents` 列表，上下键在其中循环切换焦点。

4. **组件树自绘制** — `Form::show()` 遍历子组件调用 `show()`，每个组件自行负责自己的像素绘制。

5. **位图字库** — 汉字和 ASCII 字符通过二进制点阵字库文件存储，用 `MatrixBit` 按偏移读取。

6. **平台抽象** — 通过 `#ifdef USE_SIMULATOR` / `PLATFORM_SUN8I` 等宏实现 PC 模拟器和不同硬件平台的兼容。

7. **安全机制** — `SECURELEVEL` 枚举控制窗体访问权限，`PassChkForm` 提供密码保护和防暴力破解（限次锁定）。

8. **Bresenham 算法** — 斜线绘制使用经典 Bresenham 算法，分 8 象限处理，保证任意角度的直线都能正确绘制。
