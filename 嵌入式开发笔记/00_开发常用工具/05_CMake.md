# CMake概述
本章主要是个人对https://www.bookstack.cn/read/CMake-Cookbook/README.md一些知识的总结，以及难以理解的地方进行一个通俗易懂的个人讲解，其中会剔除一些本人项目当中未使用的内容，而主要是注重遇到过的和比较重要的部分。此章节的目录将会参考该网址目录进行排列。
		`CMakeLists.txt`文件类似于`Shell`脚本，里面写的都是我们想要执行的`CMake`命令。`CMake `主要是一个配置系统(构建系统生成器), 而不是一个构建系统。
		CMake 的主要工作是生成各种构建系统可以使用的配置文件，例如生成 Makefile (Unix)、Visual Studio 项目文件 (.sln) 等，处理依赖关系、编译选项、链接选项等配置。真正的构建系统是:`Make`、`Ninja`、`Visual Studio` `MSBuild`、`Xcode`等。CMake 会自动将这些高层次的配置文件转换为具体平台上的构建系统配置文件，如在Unix上转为Makefile。

------

# CMake运行的三个步骤

## 1.配置（CMake配置阶段）

就像查看食谱和准备食材：

- 食谱：`CMakeLists.txt`文件就像食谱，它详细说明了如何构建你的项目。它包含了所有必要的指令，比如需要哪些库（食材）、如何编译源代码（烹饪步骤）等。
- 食材清单：`find_package`和`find_library`等命令就像查找食材清单，它们帮助你找到项目需要的外部库和依赖。

- 准备食材：`add_definitions`、`add_compile_options`等命令用来设置编译器的标志和定义，就像根据食谱调整食材的处理方式（比如切丁、切片）。


通俗来说：配置阶段就是告诉 CMake 你的项目需要什么，以及如何构建它。CMake 会根据`CMakeLists.txt`文件中的指令，生成一个适合你操作系统和编译器的构建系统（比如 Makefile 或 Visual Studio 项目文件）。

## 2.构建（CMake构建阶段）

就像按照食谱烹饪：

- 烹饪：一旦食谱（`CMakeLists.txt`）和所有食材（依赖库）都准备好了，你就可以开始烹饪（构建）了。这就是`cmake --build`命令做的事情，它会调用生成的构建系统（比如 make 或 ninja）来编译源代码，链接库，生成可执行文件或库文件。

- 调整火候：在构建过程中，你可以设置不同的构建类型（比如 Debug 或 Release），这就像调整烹饪的火候，以适应不同的需求（比如开发时需要更多的调试信息，而发布时需要优化性能）。


通俗来说：构建阶段就是实际编译和链接你的代码的过程。CMake 在这个阶段不会做任何决定，它只是调用配置阶段生成的构建系统来完成实际的构建工作。

## 3.安装（CMake安装阶段）

就像装盘上桌：

- 装盘：构建完成后，你的项目可能需要安装到系统中，这样其他程序就可以使用它了。`cmake --install`命令就像装盘上桌的过程，它会将可执行文件、库文件、头文件等安装到指定的位置。

- 调料：在安装过程中，你可能还需要设置环境变量（比如`PATH`）或者配置文件，这就像在装盘时添加一些调料，确保用户可以轻松地使用你的程序。


通俗来说：安装阶段是将构建好的项目部署到系统中的过程。这通常涉及到复制文件到正确的位置，并确保用户可以轻松地访问和使用它们。

## 4.总结

- 配置阶段：告诉 CMake 你的项目需要什么，以及如何构建它。

- 构建阶段：实际编译和链接代码，生成可执行文件或库文件。

- 安装阶段：将构建好的项目部署到系统中，供用户使用。

------

# CMake生成的文件介绍

## 1.CMakeCache.txt 

`CMakeCache.txt`是 执行`cmake`生成的一个缓存文件，存储了` CMake `构建过程中的各种变量和设置，存储` CMake` 运行时的配置选项缓存变量值以加速后续构建、记录项目的基本设置。

## 2.cmake_install.cmake

`cmake_install.cmake` 是执行`cmake`生成的一个安装脚本文件。主要用于处理项目的安装规则。定义如何安装项目的文件（可执行文件、库文件、头文件等）、指定安装位置、设置安装权限、处理运行时依赖。

- 当运行 `cmake `命令时自动生成。
- 当执行 make install 或 `cmake --install .` 时被调用。

------

# CLI（命令行界面）

要配置项目并生成构建器，我们必须通过命令行界面(CLI)运行`cmake`命令。

## 1.命令语法格式

```shell
cmake [选项] <源码目录> -B <构建目录>
#源码目录是CMakeLists.txt所在的目录
#构建目录是存放cmake构建过程中生成的目录
cmake [options] <path-to-source>
cmake [options] <path-to-existing-build>
cmake [options] -S <path-to-source> -B <path-to-build>
    	
 [options]  	
    	-S <DirName>     #表示DirName目录中搜索根CMakeLists.txt文件,由于顶层CMakeLists.txt与源码目录是同一个目录下。
    	-B <DirName>	 #告诉CMake在一个名为DirName的目录中生成所有的文件。
    	-G <GName> 		#切换生成器，如切换成makefile生成器，Ninja生成器等。
    	--build .		#构建示例项目
```

## 2.常用选项说明

### 设置`CMake`变量

```shell
-D<变量>=<值>

#指定C++标准
cmake . -DCMAKE_CXX_STANDARD=11
#指定交叉编译工具链文件,用于ARM平台编译
-DCMAKE_TOOLCHAIN_FILE=./CMake/arm-linux-gnueabihf.cmake    


#交叉编译: 使用CMAKE_TOOLCHAIN_FILE指定工具链进行跨平台编译

#构建类型: 通过CMAKE_BUILD_TYPE设置,常见值有:
					Debug: 调试版本
					Release: 发布版本
					RelWithDebInfo: 带调试信息的发布版本	
					MinSizeRel: 最小体积的发布版本
```

### 指定生成器类型

```shell
-G Ninja        #指定生成器为Ninja构建系统,比make更快
```

### 指定源码目录

```shell
-S .           # 指定源码目录为当前目录
```

###  指定构建目录

```shell
-B ./build/$ildType    # 指定构建目录CMakeCache.txt 文件
```

## 3.完整示例

```shell
-G Ninja                #指定生成器为Ninja构建系统,比make更快

-DToolChainRootPath=$ToolChainRoot    #设置工具链根路径变量

-DCMAKE_TOOLCHAIN_FILE=./CMake/arm-linux-gnueabihf.cmake    #指定交叉编译工具链文件,用于ARM平台编译

-DBoost_NO_BOOST_CMAKE=on             # 禁用Boost的CMake配置
-DBOOST_ROOT=$ToolChainRoot/boost_1_71_0   # 指定Boost库路径
-DBoost_ARCHITECTURE=-a32             # 指定Boost架构为32位

#设置C编译器标志,指定执行字符集为GB18030
-DCMAKE_C_FLAGS="-fexec-charset=GB18030"   

#设置C++编译器标志,启用C++14标准并指定字符集
-DCMAKE_CXX_FLAGS="-std=c++14 -fexec-charset=GB18030"

-DCMAKE_BUILD_TYPE=$BuildType         # 设置构建类型(Debug/Release等)

-S .           # 指定源码目录为当前目录
-B ./build/$buildType    # 指定构建目录即存放CMakeCache.txt等生成的文件的目录
```

1. 构建系统生成路径
   当执行完CMake命令后，会在CMake命令执行的目录【不是CMakeLists.txt所在目录】生成一些中间文件比如`CMakeCache.txt`，`Makefile`,`cmake_install.cmake`等文件。

   ```bash
   cmake -Bbuild
   ```

------

# CMake基础命令（CMakeLists.txt）

## 注释

### 行注释

```cmake
#这是一个行注释
```

### 多行（块）注释

```cmake
#[[多行注释，....，.....]]
```

## 指定最低版本

```cmake
cmake_minimum_required(VERSION 3.0)
#指定CMake的一个最低版本要求，可选，非必须，但不加会有警告
#最好还是写上，避免因主机上安装的cmake版本过低而导致的错误
```

## 指定工程名

定义工程名称，并可指定工程的版本、工程描述、web主页地址、所支持的语言（即用哪种语言去构建，默认是所有语言），如果不需要这些可以省略，只需要指出工程名即可。

```cmake
project(<project name>
		[VERSION <major>[.<minor>[.<path>]]]
		[DESCRIPTION <project_description_string>]
		[HOMEPAGE_URL <url-string>]
		[LANGUAGE <language-name>]#基于什么语言去构建，这也构建配置文件就可以指定用什么编译器
		)
```

## 生成可执行程序

```cmake
add_executable(app main.c add.c sub.c)
#或者
add_executable(app main.c;add.c;sub.c)
```

## 定义系统/自定义变量

### 为自定义变量赋值

```cmake
#语法：
set(VAR [VALUE] [CACHE TYPE DESCRIPTION[FORCE]])
#举例：
set(MyVar main.c add.c sub.c)
add_executable(app ${MyVar})
#注意：
#利用set赋值的变量值一切皆为字符串类型。
```

### 指定C++标准

```cmake
#指定C++的标准。如设置为C++11，C++12
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD 12)

#另外一种指定使用的C++标准，直接在命令行执行CMake命令时后面接参数
cmake ./ -DCMAKE_CXX_STANDARD=11
```

### 指定可执行文件的输出路径

```cmake
set(home /home/linux/demo)
set(EXECUTABLE_OUTPUT_PATH ${home}/dir )
```

## 搜索文件

### aux_source_directory

```cmake
#查找某个目录下的所有源文件
aux_source_directory(<dir> <var>)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_LIST)
```

### file

```cmake
file(GLOB/GLOB_RECURSE 变量名 要搜索的文件路径和文件类型)

#GLOB:将指定目录下搜索到的满足条件的所有文件名生成一个列表，并将其存储到变量中
#GLOB_RECURSE:递归搜索指定目录，指定目录下搜索到的满足条件的所有文件名生成一个列表，并将其存储到变量中

file(GLOB MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
```

## 指定头文件目录

```cmake
include_directories(<dir>)
 
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
```

## 库生成

### 静态库

```cmake
add_library(库名称 STATIC 源文件1 [源文件2] ...)

#在linux中，静态库名称分为三部分：lib+库名字+.a,此处只需要指定出库的名字就可以，另外两部分会自动生成。
```

### 动态库

```cmake
add_library(库名称 SHARED 源文件1 [源文件2] ...)

#在linux中，动态库名称分为三部分：lib+库名字+.so,此处只需要指定出库的名字就可以，另外两部分会自动生成。
```

### 指定库文件输出路径

```cmake
set(home /home/linux/demo/lib) 
set(LIBRARY_OUTPUT_PATH ${home}/dir)
```

## 链接库

### 指定库路径

```cmake
#如果库是系统的，可以直接写，但如果是自定义的，要指定自定义库路径

link_directories(<lib_path>)
```

### 链接静态库（编译时链接）

```cmake
link_libraries(<static lib> [<sttaic lib> ...])
#指定出要连接的静态库名，可以是全名libxxx.a也可以是掐头去尾之后的名字xxx。
```

### 链接动态库（运行时链接）

```cmake
target_link_libraries(<target> <PRIVETE | PUBLIC | INTERFACE> <item> ...
[<target> <PRIVETE | PUBLIC | INTERFACE> <item> ...]
...)

#target:指定要加载库的文件的名字，可以是一个源文件，动态库文件，可执行文件


#连接权限
target_link_libraries(A B C)
target_link_libraries(D A)

#PUBLIC:D能用A，也能用B C里定义的函数
#PRIVATE：D能用A,不能用B C里定义的函数
#INTERFACE：虽然A能用 B C里的函数，但它并不知道所用的函数是属于B还是C，可以说A只知道有这么几个函数可以用，并不知道有些什么库
```

MODULE：又为DSO组。与  SHARED  库不同，它们不链接到项目中的任何目标，不过可以进行动态加载。该参数可以用于构建运行时插件。

```cpp
// 动态加载插件
    void* handle = dlopen("./plugins/libmath_plugin.so", RTLD_LAZY);
    if (!handle) {
        printf("无法加载插件: %s\n", dlerror());
        return 1;
    }
```

cmake --help-variable-list	这个命令会列出 CMake 中所有预定义的变量 

## 常用字符串操作

### set设置

```cmake
set(MyVar main.c add.c sub.c)
```

### list列表

```cmake
#1.向列表末尾添加元素
list(APPEND MY_LIST element1 element2 ...)
 	#MY_LIST是要添加元素的列表变量，element1 element2 ... 是要添加到列表中的元素。
 	
#2.向列表开头添加元素	
list(PREPEND MY_LIST element1 element2 ...)

#3.从列表中移除指定的元素
list(REMOVE_ITEM MY_LIST element1 element2 ...)

#4.从列表中移除指定位置的元素
list(REMOVE_AT MY_LIST index1 index2 ...)

#5.获取字符串长度
list(LENGTH MY_LIST len)#len存放获取到的长度结果

#6.按索引取值
list(GET <list> <index1> [<index2> ...] <out_var>)
#out_var存放结果,可以是列表，也可以是元素	

#7.用指定连接符连接多个字符串为一个字符串
list: abc 123 ABC
list(JOIN <list> "xxx"  <out_var>)
out_var:abcxxx123xxxABC
```

## 运行时链接路径

- 在安装阶段设置`RPATH`可以指定最终用户环境中库文件的实际位置，这增加了软件的灵活性和可移植性。例如，库文件可能被安装到标准系统库目录,如/usr/lib或/usr/local/lib,这些路径在大多数Linux系统上是标准的。

- 如果在构建阶段就设置了`RPATH`,那么软件可能只能在特定的构建机器上运行,因为它依赖于构建时的路径,这限制了软件的可移植性。

- 举例：

  ```cmake
  set(CMAKE_SKIP_BUILD_RPATH TRUE)#构建时没有标签
  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)#构建完后为安装准备标签
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  list(APPEND CMAKE_INSTALL_RPATH "$ORIGIN" "$ORIGIN/../lib" "/lib/hal_lib")#为安装准备标签的标签内容
  ```

------

# CMake嵌套（多个CMakeLists.txt）

## 工程目录举例

```shell
test/
---|CMakeLists.txt#主编译规则
---|main.cpp
---|src/
------|func.cpp
------|func.h
------|dll0/
---------|CMakeLists.txt#子编译规则
---------|func0.h
---------|func0.cpp
```

## 主编译规则

定义的变量可以供子节点使用，相当于全局变量。

```cmake
#（生成可执行文件testc）

cmake_minimum_required(VERSION 3.2)
project(testc)
#执行子编译规则，编译子模块
add_subdirectory(src/dll0)

#本规则编译器配置，主要是各类编译选项
set(CMAKE_CXX_STANDARD 11)
add_compile_options(/MD)

#搜索与本规则编译相关的头文件以及源文件
include_directories(./src ./scr/dll0)
# ./ -- 指定要搜索源文件（.c .cpp .cxx）的目录。
# DIRSRCS -- 存储找到的源文件列表的变量名。
#aux_source_directory命令不会递归地搜索子目录中的源文件。如果需要递归地收集所有子目录中的源文件，需要使用file(GLOB_RECURSE ...)   命令
aux_source_directory(./ DIRSRCS)

add_excutable(testc main.cpp src/func.cpp)

#链接,用于为目标（可执行文件或库）指定链接库。这个命令告诉 CMake，在构建过程中，需要将指定的库链接到目标上。这包括运行时库、接口库以及其他目标（如其他可执行文件或库）。

#1)PRIVATE：只有目标本身会链接这些库。
#2)PUBLIC ：目标和所有依赖该目标的目标都会链接这些库。
#3)INTERFACE：只有依赖该目标的目标会链接这些库，目标本身不会链接这些库。
target_link_libraries(testc PRIVATE dll0)
```

## 子编译规则

定义的变量可以供子节点和自己使用，不能给父或祖先节点使用，相当于局部变量。

```cmake
#(生成动态库文件dll0)

aux_source_directory(. DIRSRCS)

#指定链接依赖库,不推荐使用，ink_libraries()会影响项目中之后创建的所有目标，这可能会导致不必要的链接，特别是在大型项目中。
link_libraries(${all_libs})#为项目中所有可执行文件链接all_libs中包含的库

add_library(dll0 SHARED ${DIRSRCS})#生成动态库

#将库安装到指定路径/usr/bin下
install(TARGETS dll0 DESTINATION /usr/bin)
```

------

# CMake调试打印

在cmake中可以用用户显示一条消息

```cmake
message([STATUS|WARNING|AUTHOR_WARNING|FATAL_ERROR|SEND_ERROR] "message to display" ...)

#消息等级：
#无:重要消息
#STATUS:非重要消息，一般消息
#WARNING:CMake警告，会继续执行
#AUTHOR_WARNING:CMake警告，会继续执行
#SEND_ERROR:CMake错误，继续执行，但是会跳过生成步骤
#FATAL_ERROR:CMake错误，直接被终止
```

------

# CMake自定义宏

```c
int main(int argc,char *argv[])
{
    ...;
    ...;
    #ifdef DEBUG
		printf("DEBUG\n");
	#endif
    ...;
}

```

```cmake
add_definition(-D宏名字)

#example
add_definition(-DDEBUG)
#通过下面的宏定义，上诉的DEBUG语句就会被输出出来
```

------

# CMake常用内置宏

参考链接：https://blog.csdn.net/q8250356/article/details/95660067

| 变量名                                      | 说明                                                         | 备注                                                 |
| ------------------------------------------- | ------------------------------------------------------------ | ---------------------------------------------------- |
| CMAKE_CXX_STANDARD                          | 设置CXX标准98,11,14等                                        | 对VC编译器无效                                       |
| CMAKE_BINARY_DIRP                           | 这个变量在多个项目（子项目）被包含在单个构建树中时特别有用，因为它始终指向顶级项目的构建目录。当需要引用全局构建目录中的文件或目录时使用，比如全局构建配置文件或安装目标利用此变量。 |                                                      |
| PROJECT_BINARY_DIR                          | 这个变量在每个 CMakeLists.txt 文件的作用域内都是唯一的，指向包含该 CMakeLists.txt 文件的目录的构建目录。当需要引用与特定项目相关的构建目录中的文件或目录时使用，比如项目特定的构建配置文件利用此变量。 |                                                      |
| <projectname>_BINARY_DIR                    | <projectname> 是通过project() 命令指定的项目名称。这个变量指向与特定项目相关的构建目录。当在大型项目中使用多个子项目，并且需要引用特定子项目的构建目录时使用引用此变量。 |                                                      |
| CMAKE_CURRENT_SOURCE_DIR                    | 与 CMAKE_SOURCE_DIR （指向项目的顶层源代码目录）不同， CMAKE_CURRENT_SOURCE_DIR 指向的是当前正在处理的 CMakeLists.txt 文件的目录。 |                                                      |
| CMAKE_MODULE_PATH                           | 这个变量用来定义自己的cmake模块所在的路径。如果工程比较复杂,有可能会自己编写一些cmake模块, 这些 cmake 模块是随你的工程发布的, 为了让 cmake 在处理CMakeLists.txt时找到这些模块,需要通过SET指令,将自己的cmake模块路径设置一下。如：SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)这时候就可以通过INCLUDE指令来调用自己的模块了include(modular1). | 设置后，cmake就会在include时自动在这个路径中去找模块 |
| EXECUTABLE_OUTPUT_PATH /LIBRARY_OUTPUT_PATH | 分别用来重新定义最终结果的存放目录,前面我们已经提到了这两个变量。设置，设置LIBRARY_OUTPUT_PATH后便无需调用install()安装库文件了 | 注意，这两个变量设置后，会影响编译目标的存放路径     |
|                                             |                                                              |                                                      |
|                                             |                                                              |                                                      |



## 3.1.CMAKE_MODULE_PATH

`CMAKE_MODULE_PATH` 是一个变量,它用于指定`CMake`在查找模块文件时应该搜索的目录列表。`CMake`模块文件通常是以`.cmake `扩展名结尾的文件,它们包含可以被多个项目共享的 `CMake`代码，比如查找特定库的宏或者设置特定编译选项的函数。作用是当在`CMakeLists.txt`文件中使用`include()`   命令包含一个模块时，`CMake`会在以下路径中搜索指定的模块文件：

1. `CMake`的内置模块目录。

2. `CMAKE_MODULE_PATH`变量中列出的目录。

3. 项目源代码目录。

   ```cmake
   #CMAKE_MODULE_PATH允许项目指定额外的目录，使得CMake能够找到项目特定的模块文件，这些文件可能包含项目的配置和构建逻辑。将项目源代码目录下的CMake子目录添加到模块路径
   list(PREPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)
   #或者
   set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/path/to/extra/cmake/modules")
   
   #使用list(PREPEND ...)来将新的路径添加到  CMAKE_MODULE_PATH的开头，这样 CMake 会优先搜索这些路径下的模块文件。
   ```

## 3.2.PROJECT_BINARY_DIR

# 参考链接

https://www.bookstack.cn/read/CMake-Cookbook/README.md
