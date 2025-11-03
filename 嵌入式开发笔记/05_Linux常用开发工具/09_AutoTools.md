# 背景概念

在Linux下面如果编译一个比较大型的项目，我们可以通过Makefile的方式来完成。但是，我们又蛋疼了，Makefile拥有复杂的语法结构，甚至让人难以领会，当我们项目非常大的时候，维护Makefile会成为一件非常头疼的事情。于是我们就有了***\*autotools工具\****，专门用来生成Makefile，这个工具让我们很大程度的降低了开发的难度。Autotools并不是一个工具，而是一系列工具：

```shell
   1. autoscan
   2. aclocal
   3. autoconf
   4. autoheader
   5. automake
   #记住，这一系列工具看着复杂，最终的目标还是生成Makefile
```

Autotools 是 GNU 开发的一套构建自动化工具集，核心目标是**解决跨平台编译的兼容性问题**，让开发者能够写出 “一次编写，到处编译” 的项目代码。它通过一系列脚本工具自动生成标准化的构建流程（configure → make → make install），广泛应用于开源项目（如 GCC、Apache 等）。

# 工具集下载

```bash
# 更新包索引（可选，确保版本最新）
sudo apt update
# 安装完整工具集（包含所有核心组件及依赖）
sudo apt install autoconf automake libtool m4 -y
```

# 核心工具组成

Autotools 不是单个工具，而是由多个互补工具组成的生态，核心工具包括：

## 1.基础工具

| 工具名称     | 作用说明                                                     | 核心输出文件             |
| ------------ | ------------------------------------------------------------ | ------------------------ |
| **autoscan** | 扫描源代码，自动生成初步的 configure.ac 模板（configure.scan） | configure.scan           |
| **aclocal**  | 收集本地宏定义（如系统库、头文件检测宏），生成 aclocal.m4    | aclocal.m4               |
| **autoconf** | 处理 configure.ac 和 aclocal.m4，生成跨平台的 configure 脚本 | configure                |
| **automake** | 处理 Makefile.am，生成符合 GNU 标准的 Makefile.in（configure 的输入） | Makefile.in              |
| **libtool**  | 辅助编译动态库 / 静态库（跨平台统一库命名、链接逻辑）        | .la 库文件、libtool 脚本 |

## 2.辅助工具

|  工具名称  |                           作用说明                           |
| :--------: | :----------------------------------------------------------: |
| autoheader | 从 [configure.ac](http://configure.ac/) 中提取宏定义，生成 [config.h.in](http://config.h.in/)（用于条件编译） |
| make dist  | 基于 [Makefile.am](http://makefile.am/) 配置，自动打包源码（生成 .tar.gz 压缩包） |
| make check | 执行 [Makefile.am](http://makefile.am/) 中定义的测试用例（需配置 TESTS 变量） |

# 核心工作流程

一个标准的 Autotools 项目构建流程分为 “开发者端” 和 “用户端”，流程如下：

## 1.开发者端（编写配置文件）

```tex
Autotools 开发者端流程（步骤不可逆）：
1. 编写源代码
   ↓
2. autoscan → 生成 configure.scan
   ↓
3. 手动修改 configure.scan → 重命名为 configure.ac
   ↓
4. 编写 Makefile.am（根目录+子目录需分别编写）
   ↓
5. aclocal → 生成 aclocal.m4（收集宏定义）
   ↓
6. autoheader → 生成 config.h.in（条件编译模板）
   ↓
7. autoconf → 生成 configure（核心检测脚本）
   ↓
8. automake --add-missing → 生成 Makefile.in（构建规则模板）
   ↓
9. make dist → 打包源码（生成 .tar.gz 发布包）
```

![在这里插入图片描述](..\figure\64db33ba148e4f55a69a907713d6ef8b.png)

## 2.用户端（编译安装）

```bash
# 1. 检测系统环境，生成Makefile
./configure
# 2. 编译源码（按Makefile执行）
make
# 3. （可选）运行测试用例
make check
# 4. 安装到系统目录（默认/usr/local/）
sudo make install
# 5. （可选）卸载
sudo make uninstall
```

# 核心配置文件

Autotools 的核心是 [configure.ac](http://configure.ac/) 和 [Makefile.am](http://makefile.am/)，以下是详细编写规则：

## 1.configure.ac（核心检测配置）​

configure.ac是 Autoconf 的输入文件，用于定义**系统检测逻辑**（如编译器、库、头文件是否存在），语法基于 M4 宏（以 AC_ 或 AM_ 开头）。

### 1.基础模板（最小可用版）

```SHELL
# 1. 初始化：指定Autoconf版本、包名、版本号、联系方式

AC_PREREQ([2.69])  # 要求Autoconf最低版本
AC_INIT([myproject], [1.0], [author@example.com])

# 2. 指定源码目录（.表示当前目录）
AC_CONFIG_SRCDIR([src/main.c])  # 验证源码存在（防止误执行）

# 3. 初始化Automake（必须，启用Automake支持）
AM_INIT_AUTOMAKE([-Wall -Werror foreign])  # foreign：不强制GNU标准文件（如NEWS、README）


# 4. 检测编译器（C语言，若为C++则用AC_PROG_CXX）
AC_PROG_CC  # 检测C编译器（gcc/clang等）
AC_PROG_CC_C99  # 要求C99标准


# 5. （可选）检测库/头文件
AC_CHECK_HEADERS([stdio.h stdlib.h])  # 检测系统是否有这些头文件
AC_CHECK_LIB([m], [sqrt])  # 检测数学库（libm.so）及sqrt函数



# 6. （可选）生成config.h（用于条件编译）
AC_CONFIG_HEADERS([config.h])


# 7. 指定输出文件（Makefile.in → Makefile）
AC_CONFIG_FILES([Makefile src/Makefile])  # 若有子目录，需列出子目录的Makefile



# 8. 结束检测，生成configure脚本
AC_OUTPUT
```

### 2.常用宏说明

| 宏名称                          | 作用                                                         |
| ------------------------------- | ------------------------------------------------------------ |
| AC_PREREQ([版本])               | 声明 Autoconf 最低版本                                       |
| AC_INIT([包名], [版本], [邮箱]) | 定义项目基本信息                                             |
| AM_INIT_AUTOMAKE([选项])        | 初始化 Automake，选项：- Wall：开启编译器警告- Werror：警告视为错误- foreign：宽松模式（不要求 GNU 文档） |
| AC_PROG_CC/AC_PROG_CXX          | 检测 C/C++ 编译器                                            |
| AC_CHECK_HEADERS([头文件列表])  | 检测系统是否存在指定头文件                                   |
| AC_CHECK_LIB([库名], [函数名])  | 检测系统是否存在指定库及函数                                 |
| AC_CONFIG_HEADERS([config.h])   | 生成 config.h.in（条件编译用）                               |
| AC_CONFIG_FILES([文件列表])     | 指定需要生成的 Makefile（对应 Makefile.in）                  |

## 2.Makefile.am（构建规则配置）​

[Makefile.am](http://makefile.am/) 是 Automake 的输入文件，用于定义**编译规则**（如源码文件、目标文件、安装路径等），语法简洁，Automake 会自动转换为标准 [Makefile.in](http://makefile.in/)。

### 1.基础目录结构（示例）

假设项目结构如下：

```shell
myproject/
├── configure.ac
├── Makefile.am
└── src/
	   ├── main.c
	   ├── utils.c
       └── Makefile.am
```

根目录Makefile.am:

```makefile
# 1. 定义子目录（Automake会递归处理子目录的Makefile.am）
SUBDIRS = src


# 2. （可选）指定打包时包含的额外文件（如文档、配置文件）
EXTRA_DIST = README.md LICENSE

# 3. （可选）定义卸载规则（若子目录有安装文件，需手动添加）
uninstall-local:
	rm -rf $(prefix)/share/myproject
```

子目录 src/Makefile.am:

```makefile
# 1. 定义可执行文件名称（最终生成的二进制文件）
bin_PROGRAMS = myapp

# 2. 定义可执行文件依赖的源码文件（.c/.cpp文件）

myapp_SOURCES = main.c utils.c

# 3. （可选）定义编译选项（如优化级别、警告）

myapp_CFLAGS = -O2 -Wall -Wextra

# 4. （可选）定义链接选项（如依赖的库）

myapp_LDADD = -lm  # 链接数学库（对应configure.ac中的AC_CHECK_LIB([m])）



# 5. （可选）定义安装路径（默认bin_PROGRAMS安装到$(prefix)/bin）
# 若要安装到其他目录，可替换bin为sbin、libexec等（如sbin_PROGRAMS安装到/sbin）



# 6. （可选）定义头文件安装（若需要对外提供头文件）

include_HEADERS = utils.h  # 安装到$(prefix)/include

```

### 2.关键变量说明

| 变量类型 | 示例            | 作用                                                         |
| -------- | --------------- | ------------------------------------------------------------ |
| 目标类型 | bin_PROGRAMS    | 生成可执行文件，安装到 $(prefix)/bin                         |
|          | lib_LTLIBRARIES | 生成动态库（libtool），安装到 $(prefix)/lib                  |
|          | include_HEADERS | 安装头文件到 $(prefix)/include                               |
| 源码依赖 | xxx_SOURCES     | 指定目标 xxx 依赖的源码文件（如 myapp_SOURCES）              |
| 编译选项 | xxx_CFLAGS      | C 编译器选项（如 -O2 -Wall）                                 |
|          | xxx_CXXFLAGS    | C++ 编译器选项                                               |
| 链接选项 | xxx_LDADD       | 链接依赖库（如 -lm -lpthread）                               |
| 安装路径 | prefix          | 安装根目录（默认 /usr/local，可通过 ./configure --prefix=/opt/myapp 修改） |
| 额外文件 | EXTRA_DIST      | 源码打包时包含的非编译文件（如文档、LICENSE）                |

### 3.核心预设目录对照表

| 目标类型前缀 | 对应系统目录（默认，基于 prefix=/usr/local） | 适用场景                                       | 示例配置                                                     |
| ------------ | -------------------------------------------- | ---------------------------------------------- | ------------------------------------------------------------ |
| bin_         | $(prefix)/bin → /usr/local/bin               | 普通用户可执行程序（如 ls、git）               | bin_PROGRAMS = myapp（安装到 /usr/local/bin/myapp）          |
| sbin_        | $(prefix)/sbin → /usr/local/sbin             | 系统管理类程序（如 ifconfig、service）         | sbin_PROGRAMS = mysyscmd（安装到 /usr/local/sbin/mysyscmd）  |
| libexec_     | $(prefix)/libexec → /usr/local/libexec       | 程序内部依赖的辅助执行文件（不直接给用户调用） | libexec_PROGRAMS = myhelper（安装到 /usr/local/libexec/myhelper） |
| lib_         | $(prefix)/lib → /usr/local/lib               | 静态库（.a）、动态库（.so）                    | lib_LIBRARIES = libmystatic.a（静态库，安装到 /usr/local/lib） |
| libexec_     | $(prefix)/libexec → /usr/local/libexec       | 程序内部辅助工具（如插件、子进程脚本）         | libexec_SCRIPTS = myplugin.sh（安装辅助脚本）                |
| include_     | $(prefix)/include → /usr/local/include       | 对外提供的头文件（供其他项目引用）             | include_HEADERS = mylib.h（安装到 /usr/local/include/mylib.h） |
| share_       | $(prefix)/share → /usr/local/share           | 架构无关的共享文件（文档、配置模板、语言包）   | share_DOCS = README.md（安装文档到 /usr/local/share/doc/myproject） |

### 4.静态库 / 动态库编译

若项目需要生成库文件（.so/.a），需使用 libtool，配置如下（修改 src/Makefile.am）：

```makefile
# 1. 启用libtool（必须）

LT_INIT([dlopen])  # dlopen：支持动态加载


# 2. 定义库文件名称（前缀lib会自动添加，最终生成libxxx.la）

lib_LTLIBRARIES = libxxx.la


# 3. 库的源码文件
libmylib_la_SOURCES = utils.c


# 4. 编译选项

libmylib_la_CFLAGS = -O2 -Wall


# 5. 库版本号（遵循libtool版本规则：current:revision:age）
# 规则：若接口兼容，age++；若接口不兼容，current++，age=0
libmylib_la_LDFLAGS = -version-info 1:0:0


# 6. 安装头文件（供其他项目使用）
include_HEADERS = utils.h
```

编译后会生成：

- libxxx.la：libtool 中间文件（记录库依赖）；

- libmylib.so.1.0.0：动态库（实际运行用）；

- libmylib.a：静态库（可选，需启用 --enable-static）。

# 实战案例

从零构建一个 Autotools 项目

## 1.创建项目结构

```bash
mkdir -p myproject/src

cd myproject
```

## 2.编写源代码

src/main.c：

```c
#include <stdio.h>
#include "config.h"  // 条件编译头文件（autoheader生成）
int main() {
	
    printf("Hello Autotools! Project version: %s\n", PACKAGE_VERSION);
#ifdef HAVE_STDLIB_H
	printf("stdlib.h is available!\n");
#endif
	return 0;
}
```

src/utils.c：

```c
#include <stdlib.h>
int add(int a, int b) {
	return a + b;
}
```

src/utils.h：

```c
#ifndef UTILS_H
#define UTILS_H
int add(int a, int b);
#endif
```



## 3.生成并修改 configure.ac

```bash
# 1. 扫描源码，生成configure.scan
autoscan

# 2. 重命名为configure.ac并修改
mv configure.scan configure.ac
```
修改后的configure.ac：
```bash
AC_PREREQ([2.69])

AC_INIT([myproject], [1.0], [author@example.com])

AC_CONFIG_SRCDIR([src/main.c])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_PROG_CC

AC_PROG_CC_C99

AC_CHECK_HEADERS([stdio.h stdlib.h])

AC_CONFIG_HEADERS([src/config.h])  # config.h生成到src目录

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
```



## 4.编写 Makefile.am

根目录 Makefile.am：

```makefile
SUBDIRS = src
EXTRA_DIST = README.md LICENSE
```

src/Makefile.am：

```makefile
bin_PROGRAMS = myapp

myapp_SOURCES = main.c utils.c

myapp_CFLAGS = -O2 -Wall

myapp_LDADD = -lm

include_HEADERS = utils.h
```

## 5.生成构建文件

```bash
# 1. 生成aclocal.m4（收集宏）
aclocal

# 2. 生成config.h.in（条件编译模板）
autoheader

# 3. 生成configure脚本

autoconf

# 4. 生成Makefile.in（Automake）

automake --add-missing  # --add-missing：自动添加缺失的辅助文件（如install-sh）
```

## 6.编译安装

```bash
# 1. 检测系统环境，生成Makefile

./configure --prefix=/opt/myapp # 自定义安装目录（默认/usr/local）

# 2. 编译
make

# 3. 安装
sudo make install

# 4. 运行测试

/opt/myapp/bin/myapp

# 输出：

	# Hello Autotools! Project version: 1.0

	# stdlib.h is available!
```



步骤 7：打包发布



bash取消自动换行复制

*# 生成源码包（myproject-1.0.tar.gz）*

make dist



五、常见问题与调试技巧

1. 配置文件语法错误​

- **问题**：autoconf 报错 syntax error near unexpected token；

- **解决**：检查 [configure.ac](http://configure.ac/) 中的宏是否闭合（如 AC_INIT 后是否有 AC_OUTPUT），宏参数是否用 [] 包裹。
- Makefile 生成失败​

- **问题**：automake 报错 required file 'xxx' not found；

- **解决**：添加 --add-missing 参数（automake --add-missing），自动生成缺失的辅助文件（如 install-sh、COPYING）。
- 库链接失败​

- **问题**：make 报错 undefined reference to 'sqrt'；

- **解决**：在 [Makefile.am](http://makefile.am/) 中添加 xxx_LDADD = -lm（链接数学库），并在 [configure.ac](http://configure.ac/) 中添加 AC_CHECK_LIB([m], [sqrt])。
- 调试技巧​

- **查看 configure 检测日志**：./configure --enable-debug 2> config.log，通过 config.log 排查检测失败原因；

- **查看生成的 Makefile**：make -n 打印编译命令（不实际执行）；

- **禁用某些检测**：./configure --without-libxxx（需在 [configure.ac](http://configure.ac/) 中配置 AC_ARG_WITH）。

六、进阶用法

1. 条件编译（基于系统特性）​

在 [configure.ac](http://configure.ac/) 中添加自定义检测：



m4取消自动换行复制

\# 检测是否支持64位

AC_CHECK_SIZEOF([long], 4)

if test $ac_cv_sizeof_long -eq 8; then

​    AC_DEFINE([SUPPORT_64BIT], [1], [Support 64-bit integers])

fi



在源码中使用：



c取消自动换行复制

\#include "config.h"

\#ifdef SUPPORT_64BIT

​    printf("64-bit supported\n");

\#endif



2. 自定义编译选项​

在 [configure.ac](http://configure.ac/) 中添加选项：



m4取消自动换行复制

\# 启用调试模式：./configure --enable-debug

AC_ARG_ENABLE([debug],

​    [AS_HELP_STRING([--enable-debug], [Enable debug mode])],

​    [debug=$enableval], [debug=no])



if test "x$debug" = "xyes"; then

​    CFLAGS="$CFLAGS -g -O0"

​    AC_DEFINE([DEBUG], [1], [Debug mode enabled])

fi



3. 跨平台库依赖​

使用 PKG_CHECK_MODULES 检测系统库（如 GTK、JSON-C）：



m4取消自动换行复制

\# 检测json-c库

PKG_CHECK_MODULES([JSONC], [json-c >= 0.15])

CFLAGS="$CFLAGS $JSONC_CFLAGS"

LIBS="$LIBS $JSONC_LIBS"



对应的 [Makefile.am](http://makefile.am/) 中无需额外配置，PKG_CHECK_MODULES 会自动添加编译 / 链接选项。

4. 生成共享库并安装​



makefile取消自动换行复制

*# 根目**录**Makefile.am*

SUBDIRS =</doubaocanvas>
