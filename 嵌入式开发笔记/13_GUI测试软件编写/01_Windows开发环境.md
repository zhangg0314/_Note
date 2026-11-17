# 开发工具

**Windows SDK** 提供 Windows API 的开发材料，包含各种Windows API 库和操作文档。编译器可以选择**GNU g++** 和 **MSVC cl.exe** 两种 C++ 编译器，编辑器与编译器可自由组合：例如用 VS Code 写 C++，再用 Build Tools 的命令行编译。

- **Visual Studio 2022** 
  带编辑、调试、项目管理功能的 IDE。
- **Visual Studio 2022 Build Tools**
  不带完整 IDE 的命令行构建工具包，可提供 MSVC 编译器、链接器等。

# 编译流程

以熟悉的Linux做对比来理解

| 环节 | Linux 常见做法 | Windows 常见做法 |
| --- | --- | --- |
| 编译器 | `g++` | MSVC `cl.exe` |
| 目标文件 | `.o` | `.obj` |
| 链接 | 通常由 `g++` 调用链接器 | 构建脚本直接调用 `link.exe` |
| 可执行文件 | ELF 文件，通常无扩展名 | PE 格式 `.exe` |
| 动态库 | `.so` | `.dll` |
| 常见构建方式 | Shell、Make、CMake | 自己编写的 `.cmd` 脚本，也可以用 MSBuild 或 CMake |

例如，Linux 中的 `g++ -std=c++17 -c main.cpp -o main.o`，对应 MSVC 的 `cl /std:c++17 /c main.cpp`；再由链接器把目标文件和所需库组合为程序。这里的 `-c` 与 `/c` 都表示先只编译、不链接。

# Windows SDK

Windows SDK 主要包括：

- **头文件**，如 `windows.h`、`winsock2.h`：声明 Windows API 的函数和类型。
- **导入库**，如 `ws2_32.lib`：链接时让程序知道调用哪个系统 DLL；它通常不是函数实现本身。
- **开发工具**，如资源编译器 `rc.exe`，以及一些特定 Windows 功能的开发文件。

实际运行 Winsock 函数的 `ws2_32.dll` 属于 Windows 系统。应用一般调用 **Win32 API**，不是直接发出内核系统调用。Windows SDK 与 Linux 上的系统开发包在用途上相近：提供编译所需的声明、链接材料和工具。

注意事项：

测试软件也常使用C#编写，而C# 程序通常使用 **.NET SDK** 构建，并通过 .NET API 与系统交互。

例如 `File.ReadAllText()` 或 `System.Net.Sockets` 最终由 .NET 在底层使用操作系统功能。因此，普通 C# 程序通常不需要自己包含 Windows SDK 的 C++ 头文件或链接 `.lib`。开发 C# 可以使用 Visual Studio IDE，也可以用 VS Code 加 `dotnet build`。如果 .NET 没有封装所需的 Windows 功能，C# 仍可通过 **P/Invoke** 调用系统 DLL。C++ 配合 Windows SDK 通常是直接调用 Win32 API，C# 通常先调用 .NET 封装；两者最终都可使用 Windows 功能。“更接近底层”指更直接面对原生 API，不等于直接操作内核或硬件。

# 常见技术组合

- **编程语言**负责业务逻辑
- **GUI 框架**负责窗口控件
- **编译/运行环境**负责生成和运行程序
- **系统或第三方库**负责串口、TCP、USB、数据库等能力。

| 技术组合 | 常用工具 | 平台 | 适合的测试软件 | 主要代价 |
| --- | --- | --- | --- | --- |
| **C++ + Win32 API** | MSVC/Build Tools 或 MinGW-w64，Windows SDK/对应 API 库 | Windows | 已有 C/C++ 协议代码、需要直接调用 Windows API、追求较小的原生程序 | 界面代码较繁琐，平台依赖明显 |
| **C++ + Qt Widgets** | Qt、CMake、MSVC 或 MinGW-w64；Linux 可用 g++ | Windows、Linux、macOS | 需要跨平台桌面界面，或要复用 C/C++ 协议库 | 需学习 Qt，对应平台需打包 Qt 库 |
| **C# + WinForms / WPF** | .NET SDK、Visual Studio 或 VS Code | Windows | 内部使用的设备配置、串口、TCP、日志查看工具，要求快速做出 Windows 界面 | WinForms/WPF 界面不能直接搬到 Linux；调用现有 C++ 库需互操作 |
| **C# + Avalonia** | .NET SDK、Avalonia、Visual Studio 或 VS Code | Windows、Linux、macOS | 熟悉 C# 且需要跨平台桌面界面 | 多一个 UI 框架和打包体系需要学习 |
| **Python + PySide6 / Tkinter** | Python、pip、PySide6 或标准库 Tkinter；可用 PyInstaller 打包 | Windows、Linux、macOS | 快速做协议验证、串口调试、数据采集和小型内部工具 | 发布时需处理 Python 解释器与依赖；大量计算或复杂原生集成需另行设计 |
| **Web 界面 + 本地服务** | HTML/CSS/JavaScript 或 TypeScript；本地服务可用 Python、C#、Go、C++ 等 | 有浏览器的平台 | 多人通过浏览器查看测试状态、远程管理模拟器 | 本地串口/USB 等硬件访问通常要经过后端服务；需要处理服务启动与访问控制 |
| **Electron 桌面应用** | JavaScript/TypeScript、Node.js、Web 前端工具 | Windows、Linux、macOS | 团队熟悉 Web 技术，界面较复杂、需要桌面打包 | 安装包及内存占用通常较大 |

表中“跨平台”表示框架支持这些系统，**不保证任意现有代码无需修改就能跨平台**。若业务层直接使用 Win32、Winsock 或专用 Windows DLL，仍需做适配。Tkinter 适合简单界面；复杂表格、图形和交互通常更适合 PySide6 等框架。Qt、Avalonia、Electron 的发布包都需包含各自运行所需的组件。

### 典型功能怎么落地

| 需求 | 常见选择 |
| --- | --- |
| TCP/UDP 通信 | C++：Winsock/Qt Network；C#：`System.Net.Sockets`；Python：`socket` / `asyncio` |
| 串口通信 | C++：Win32 串口 API / Qt Serial Port；C#：`System.IO.Ports`；Python：`pyserial` |
| 保存配置和测试数据 | JSON、INI 或 SQLite；数据量和查询需求增加时再考虑数据库 |
| 多设备并发 | 后台线程、异步 I/O 或事件循环；把每台设备的连接状态、定时器和日志分开管理 |
| 发布给别人使用 | C++ 打包依赖 DLL；C# 可依赖目标机 .NET 或发布为自包含程序；Python 可用 PyInstaller；Web 方案需部署后端 |

GUI 程序要避免在**界面线程**里直接执行长时间收发或阻塞等待，否则窗口会卡住。常见结构是：**GUI（配置、启动停止、状态展示）→ 后台连接/任务管理 → 协议解析与模拟 → 配置、样本和日志**。协议处理最好能脱离界面单独运行和测试，方便以后更换 GUI 技术。

### 结合本项目如何选择

- **维护现有集中器模拟器**：继续用 C++17 + Win32 + MSVC，现有协议和构建脚本已经走通。
- **从 Linux C++ 经验迁移，准备写新的跨平台模拟器**：优先学习 C++ + Qt Widgets + CMake；先实现命令行协议核心，再加界面。
- **只面向 Windows，想尽快做出内部测试工具**：C# + WinForms 通常入门快；界面更复杂可考虑 WPF。
- **先验证设备协议或做一次性工具**：Python + PySide6/pyserial/SQLite 常能较快完成；规模扩大后再评估部署和性能。