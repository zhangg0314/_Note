# 设计特性

- **模块化设计**：将功能分离（如渲染、事件处理、布局），提高代码可维护性。
- **多态性**：通过基类指针操作不同类型的对象。
- **代码复用**：继承通用功能（如位置管理、可见性），减少重复代码。
- **分层抽象**：从底层画布到高层 `UI` 组件，形成清晰的抽象层次。

# 字体类

## 1.类介绍

定义了一个`Font`类，用于处理不同点阵字体的显示和属性管理。

## 2.字体类型

```c++
enum FONTTYPE
{
    FONT16 = 0,//汉字16 * 16,ASCII码 16 * 8 （高 * 宽）
    FONT12 = 1,//汉字12 * 12，ASCII码12 * 8  （高 * 宽）
    FONT8 = 2,//汉字无，ASCII码8 * 8  （高 * 宽）
};
```

------

# 核心类设计

## 1.基础抽象类

所有可显示对象的抽象基类，定义通用属性和方法：

```c++
/////////////////////////////////////////////////////////
// 所有可视组件的基类
////////////////////////////////////////////////////////
class ViewComponent
{
public:
    // x, y为组件的横, 纵坐标, 单位:点;
    ViewComponent(int x = 0, int y = 0, bool enabled = false, bool visible = true);
    virtual ~ViewComponent();
    virtual void show();//控制组件是否显示                         ===MatrixBit::Draw，把要显示的内容组成点阵，写入zone区
    virtual void enable();//控制组件是否响应用户交互（如选中）。	   ===label选中高亮或者Form调用show显示当前页
    virtual void disable();//控制组件是否响应用户交互（如未选中）。   ===lable未选中失亮或者Form返回到上级窗口
    virtual void setvisible(bool visible);//设置组件是否可视	   ===只有组件可视化时。调用show时才会把当前组件的点阵也加入zone区
    virtual bool visible();//控制组件是否显示。
    
    
    
    virtual void click();//处理点击事件，派生类可重写              ===按钮组件独有，当点击事件发生时的处理逻辑
    
    
    virtual void setfocus(bool state);//设置焦点状态，即选中状态
    virtual void setcanvas(class Canvas *);
    virtual void setposition(struct Position pst);
    virtual int GetHight();
    void setName(const char *txt);
    char *getName(void);
    virtual struct Point getposition();
protected:
    class Canvas *canv;//指向渲染画布的指针，用于将组件绘制到屏幕
private:
    struct Point pst; // 组件的坐标结构化值
    bool isenabled;
    bool isvisible;//可见性
};
```

## 2.窗口组件类

### 1.类介绍

`Form` 类是 LCD 显示系统中的**窗口抽象**，继承自 `ViewComponent`，负责管理子组件、处理键盘事件和维护窗口层级关系。它实现了 GUI 系统中常见的**表单 / 对话框**概念，支持组件布局、焦点管理和模态交互。菜单下的都是一个个的窗体`Form`类。由`Form`类来控制其内部组件的状态。

```c++
#ifndef _FORM_H
#define _FORM_H

#pragma interface

#include <vector>
#include <string>

#include "viewcomponent.hpp"

#include "types.hpp"
#include "label.hpp"
#include "button.hpp"
#include "securecheck.hpp"
#include "canvas.hpp"
#include "keyboardManage.hpp"

const int MAX_HINT_LEN = 30;

////////////////////////////////
// 窗口组件: 负责接收输入设备消息, 管理所属组件行为
////////////////////////////////
class Form : public ViewComponent
{
public:
    Form(Form * = NULL);
    virtual ~Form();
    virtual void show();
    virtual void fresh() {}; 		// 显示刷新
    virtual void enable();           // 窗口使能函数
    virtual void disable();          // 退回到上级窗口
    virtual void keydown(enum KEYS); // 键盘消息处理函数
    virtual void key_enter();        //"确认"按键响应涵数
    virtual void key_left();         //"左"按键响应涵数
    virtual void key_right();        //"右"按键响应涵数
    virtual void key_down();         //"上"按键响应涵数
    virtual void key_up();           //"下"按键响应涵数
    virtual bool IsLastShow()
    {
        return true;
    };
    virtual bool IsFirstShow()
    {
        return true;
    };
    void addcomponent(ViewComponent *cmpnt, bool tab); // 添加组件, tab 为真时, 添加的组件可以出于被选择状态
    void addtabcomponent(ViewComponent *cmpnt, bool tab); // 添加组件到可响应组件列表，被添加的组件一定处于components中
    void delcomponent(ViewComponent *cmpnt);              // 删除组件
    void deltabcomponent(ViewComponent *cmpnt); // 从可响应组件列表中移除组件，组件还是存在的
    void clearcomponents();                     // 清除所有组件
    void setparent(Form *);                     // 设置父窗口
    Form *getparent()
    {
        return parent;
    };
    void settaborder(unsigned int order); // 设置窗口处于被选中状态的组件, order: 组件序号
    enum SECURELEVEL getsecurelevel();    // 获得窗口的安全级别
    void nofocus();                       // 使窗口中所有组件都出于非选中状态
    unsigned int gettaborder();           // 获得处于被选择状态的组件序号
    ViewComponent *getfocusObj();         // 获得处于被选择状态的组件对象指针
    Canvas canvas;                        // 窗口画布
    static class Form *current;           // 当前激活窗口对象指针
    bool isfromchild;                     // isformchild 为真时 表示窗口由子窗口切换而来
    bool succeeded;

    void sethint(const char *txt); // 设置窗口的提示信息
    const char *gethint()          // 获得窗口的提示信息
    {
        return hint;
    }

    static int iKeyNullCount;

protected:
    Form *parent;                 // 父窗口对象指针
    enum SECURELEVEL securestate; // 窗口安全级别

    virtual void key_cancel(); //"取消"按键响应涵数
    virtual void key_null()
    {
        iKeyNullCount++;
    };                                 //"无效"按键响应函数
    static void CloseForm(Form *form); // 关闭窗体并将form 所占空间释放掉
private:
    unsigned int taborder;
    std::vector<ViewComponent *> components;//窗体有哪些组件
    std::vector<ViewComponent *> tabcomponents;//窗体的组件中有哪些可以被选择响应事件的组件
    char hint[MAX_HINT_LEN]; // 窗口的提示信息

    //  void JudgeAndSetWindow();
};
#endif
```

### 2.父窗体和子窗体

- **父窗体（Parent Window）**
  包含其他窗体或控件的容器，负责管理子窗体的布局和渲染顺序
- **子窗体（Child Window）**
  依赖于父窗体存在的窗口，如对话框、工具栏等。子窗体的位置和生命周期由父窗体管理。

### 3.为什么需要设置`parent`

`parent`指针用于：

- 确定对象在显示层级中的位置（子对象坐标通常相对于父对象）。
- 实现事件冒泡（如点击事件从子对象向上传递到父对象）。
- 管理对象生命周期（父对象销毁时自动销毁子对象）。

### 4.核心作用

1. **组织 UI 元素**
   将相关的控件（如输入框、按钮、列表）组合在一起，形成功能完整的界面单元（如登录表单、设置面板）。
2. **隔离交互逻辑**
   每个窗体可独立处理自身的事件（如键盘输入、触摸点击），避免与其他窗体冲突。
3. **实现界面层级**
   通过父 - 子窗体关系，构建复杂的导航结构（如主菜单→子菜单→详情页）。
4. **支持模态操作**
   模态窗体（如确认对话框）会阻止用户与其他界面交互，确保操作的完整性。

### 5.常见窗体组件示例

1. **登录窗体**
   包含用户名 / 密码输入框、登录按钮，验证用户身份。
2. **消息对话框**
   显示提示信息、警告或错误，通常包含 “确认” 按钮。
3. **设置面板**
   分组展示设备或应用设置选项，支持分类配置。
4. **数据列表窗体**
   显示表格数据（如电力历史记录），支持翻页和筛选。

# 窗体中的组件类

## 1.标签组件类

### 1.类介绍

`Label`类，它继承自`ViewComponent`，用于在界面上显示文字信息。

```c++
//////////////////////////////////
// 文字组件, 显示文字信息
//
//////////////////////////////////
class Label : public ViewComponent
{
public:
    Label(unsigned int x = 0, unsigned int y = 0, const char *txt = 0);
    ~Label();
    void settext(const char *); //设置要显示的文字
    void setfmttext(const char *fmt, ...);//显示格式化字符串
    void setvalue(int value);
    void setvalue(long value);
    void setvalue(float value);
    void setvalue(double value);
    char *gettext();
    void SetFont(enum FONTTYPE type);
    void show();
    virtual void enable();
    virtual void disable();
    void setcanvas(Canvas *sender);     // 设置画布
    void setfocus(bool state);          // 使文字反白显示，即选中状态
    void setposition(struct Point pst); // 设置文字位置
    void setposition(int x, int y);
    void left();   // 靠左显示字符串
    void right();  // 靠右显示字符串
    void center(); // 居中显示字符串
    int GetHight()
    {
        return font->GetHight();
    }

    struct Point getposition()
    {
        return startpst;
    };
    void SetFontType(enum FONTTYPE);
    int GetFontHight()
    {
        return font->GetHight();
    };
    enum FONTTYPE GetFontType()
    {
        return font->GetType();
    };

private:
    struct Point startpst;
    struct Point endpst;
    char *text;
    int maxlen; // dynamic allocatimg area length
    bool focused;
    Font *font;
    //  class Canvas* canv;
};
#endif //_LABEL_HPP
```

### 2.应用示例代码

```c++
DLMSCommform::DLMSCommform(Form *p, bool fromOrder) : Screen(p),
workMode(64, 17 * 2, 5, 3),
mainIPEdt1((char *)"###.###.###.###", 28, 17 * 3),
mainPortEdt1((char *)"#####", 40, 17 * 4),
heartbeatEdt((char *)"#####", 48, 17 * 5)
{

    // 标题
    LblTitle.setposition(0, 0);//设置起始位置
    LblTitle.settext("DLMS参数设置");//设置标题
    LblTitle.center();//居中对齐
    
    addcomponent(&LblTitle, false);//DLMSCommform窗体添加标题lable组件
    
    
    canvas.SetSize(0, 160, 0, 320);
    canvas.SetWindowSize(160, 130);
    canvas.SetOrig(0, 17);
    ...
        ...
}
```

