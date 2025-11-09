# ——相关概念——

# 概念介绍

`dpkg-buildpackage` 是 `Debian/Ubuntu` 系统中用于构建软件包的核心工具，它会调用一系列底层工具（如 `dpkg-deb`、`dpkg-gencontrol`）来自动生成 `.deb` 软件包。`.deb`软件包相当于 Windows 中的 **`.msi` 或 `.exe` 安装文件**。

开发者只需要运行 `dpkg-buildpackage` 这一个命令，就能自动完成从 “源代码” 到 “可安装`.deb`包” 的全流程，其执行流程大致如下：

1. 检查打包目录是否符合规范（比如当前目录下有没有`debian/`目录和必要文件）。
2. 调用编译工具（如 `make`）将源代码编译成二进制文件。
3. 生成各种元数据文件（如 `control`、`md5sums`）。
4. 调用 `dpkg-deb` 把所有文件打包成 `.deb`。
5. 最终在工程目录的同级目录下生成 `.deb` 包、源代码包等产物。

------

# 核心功能

1. **自动化构建流程**
   从源代码生成完整的 `.deb` 包，包括编译、打包、生成控制文件等步骤。
2. **多架构支持**
   支持交叉编译（如在` x86` 机器上为 ARM 架构构建包）。
3. **签名与验证**
   支持使用` GPG `对生成的包进行数字签名，确保完整性。
   
   ```shell
   #软件包的签名与认证核心是验证软件完整性和来源合法性的安全机制。
   1.核心定义
   	软件包签名：开发者用私钥对软件包数据（或其哈希值）加密生成 “签名文件”，随软件包一同发布。
   	软件包认证：用户 / 系统用开发者公开的公钥，解密签名并校验，确认软件未被篡改且来自合法开发者。
   2.关键作用
   	防篡改：确保软件下载或传输过程中，没有被植入恶意代码、替换文件等。
   	验来源：确认软件确实来自声称的开发者，避免下载到仿冒、恶意软件。
   	抗抵赖：开发者无法否认自己发布过该版本的软件包。
   3.简单流程
   	开发者：生成密钥对（私钥 + 公钥），用私钥给软件包签名，发布软件包 + 签名文件 + 公钥。
   	用户 / 系统：获取后用公钥解密签名，同时计算软件包的哈希值，对比解密结果与哈希值是否一致，一致则认证通过。
   ```
4. **依赖检查**
   自动检查并处理构建依赖（如所需的开发库）。如果开发者的电脑上没有 `libssl` 开发版（编译时需要），工具会报错，提醒开发者安装；生成的 `.deb` 包会包含依赖信息，用户安装时，系统会自动帮其先装上 `libssl(运行时需要)`，再装程序。
   构建时依赖：
   
   ```makefile
   Build-Depends: debhelper (>= 7), autotools-dev
   ```
   
   运行时依赖：
   
   ```makefile
   Depends: ${shlibs:Depends}, ${misc:Depends}
   ```

------

# 模拟权限

## 1.先安装 fakeroot

通过 apt 快速安装：

```bash
sudo apt update
sudo apt install fakeroot
```

安装完成后，可通过`fakeroot --version`验证是否安装成功，出现版本信息则说明安装没问题。

## 2.使用 `fakeroot`

`fakeroot`最常用的场景是配合`dpkg-buildpackage`、`dpkg-deb`等工具构建 deb 包，以下是两个核心用法，覆盖不同打包阶段：

1. 搭配 dpkg-buildpackage 构建完整 deb 包

   `dpkg-buildpackage`是构建 deb 包的核心命令，构建过程中会创建`/usr/bin`、`/etc`这类系统级目录（正常用户无权限），用

   `fakeroot`包裹后可模拟权限，避免真 root 构建：

   ```bash
   # 进入打包目录（比如之前的~/dpkg/termsg目录）
   cd ~/dpkg/termsg
   # 用fakeroot执行构建，可结合--createorig解决之前orig.tar.xz的问题
   fakeroot dpkg-buildpackage --createorig
   ```

   执行后，`fakeroot`会让`dpkg-buildpackage`误以为拥有 root 权限，顺利完成系统目录创建、文件权限设置等操作，且全程不会触发真实 root 的高危权限，安全性拉满。

2. 搭配 dpkg-deb 手动打包（适合自定义打包流程）

   若通过`dh_make`生成`debian/`目录后，想手动打包测试，可先用`make install`将文件安装到临时目录，再用`fakeroot`+`dpkg-deb`打包：

   ```bash
   # 假设已编译源码，先创建临时安装目录
   mkdir -p temp_install
   # 把软件文件安装到临时目录（模拟系统安装路径）
   make install DESTDIR=./temp_install
   # 用fakeroot打包成deb包，避免创建目录/文件时权限不足
   fakeroot dpkg-deb --build temp_install termsg_1.0.0_amd64.deb
   ```

   这里`dpkg-deb --build`需要创建符合 deb 包规范的目录结构和权限，`fakeroot`会解决普通用户执行时的权限报错。

## 3.核心原理

`fakeroot`并不是真的赋予 root 权限，而是通过**拦截系统调用**实现 “权限欺骗”：

1. 当被包裹的命令（如`dpkg-buildpackage`）尝试执行需要 root 权限的操作（如创建`/usr`目录、设置文件所有者为 root）时，`fakeroot`会拦截这些请求；
2. 它仅在当前进程和子进程中 “伪造” 权限结果，让命令认为操作成功，但这些操作不会对真实系统产生 root 级别的修改；
3. 最终生成的 deb 包中，文件的所有者、目录结构等配置会被正确记录，用户安装时会按这些配置生效，而构建过程本身是安全的。

## 4.注意事项

1. `fakeroot`仅模拟文件权限、目录创建等与文件系统相关的 root 操作，无法模拟网络端口绑定、修改系统内核参数等其他 root 权限操作（这类操作也不属于 deb 包构建的常规流程）；
2. 构建 deb 包时，**优先用 fakeroot 而非 sudo**，避免误操作修改系统文件，或因权限过高导致打包文件的所有者异常，进而引发用户安装后软件无法运行的问题。

------

# ——软件安装使用——



# ——软件开发打包——

# 构建打包命令

## 1.`dh_make`命令

### 1.命令介绍

`dh_make` 是 `Debian` 打包系统中的核心工具，用于**自动生成`debian/`目录的初始骨架**。它会根据项目类型（原生包、上游源码包等）创建必要的控制文件，大幅减少手动配置的工作量。通过 `dh_make`，可以快速搭建 `Debian` 包的基础结构，然后专注于定制特定的打包规则和元数据。这大大降低了手动编写复杂控制文件的门槛。

### 2.命令格式

```bash
dh_make [选项] [包版本]

#示例：为项目创建Debian打包骨架
cd myproject/
dh_make     --single --native   
			-e your.email@example.com   
			-p termsg_1.0.0
```

### 3.核心选项

#### 1. **包类型选项**

| 选项           | 作用                                             |
| -------------- | ------------------------------------------------ |
| `--native`     | 创建原生`Debian`包（无上游源码，纯`Debian`项目） |
| `--createorig` | 自动创建上游源码 tarball（用于非原生包）         |
| `--single`     | 创建单包（而非多包）项目                         |
| `--library`    | 创建库包（自动设置合适的依赖和规则）             |

#### 2. **元数据选项**

| 选项        | 作用                                                     |
| ----------- | -------------------------------------------------------- |
| `-e <邮箱>` | 设置维护者邮箱（会写入 `debian/control` 和 `changelog`） |
| `-c <协议>` | 指定版权协议（如 `gpl2`、`mit` 等，默认 `gpl3`）         |
| `-n`        | 不创建示例文件（如 `README.Debian`）                     |

#### 3. **文件生成选项**

| 选项                        | 作用                                      |
| --------------------------- | ----------------------------------------- |
| `-p <名称>_<版本>`          | 指定包名和版本（覆盖默认值）              |
| `--createorig`              | 自动创建上游源码 tarball（用于非原生包）  |
| `--copyright-format <格式>` | 指定版权文件格式（如 `machine-readable`） |

#### 4. **构建系统选项**

| 选项          | 作用                                                      |
| ------------- | --------------------------------------------------------- |
| `--python`    | 针对 Python 项目优化（自动配置 `pybuild` 或 `dh-python`） |
| `--cmake`     | 针对 CMake 项目优化                                       |
| `--autotools` | 针对 Autotools 项目优化（`configure`/`make`）             |

#### 5. **其他常用选项**

| 选项               | 作用                             |
| ------------------ | -------------------------------- |
| `-s`               | 创建小规模包（只生成必需的文件） |
| `-y`               | 对所有询问自动回答 "yes"         |
| `--email-from-git` | 从 Git 配置中读取维护者邮箱      |

### 4.生成的关键文件

运行 `dh_make` 后，会在 `debian/` 目录下生成以下核心文件：

```plaintext
debian/
├── changelog      # 版本变更记录（初始条目）
├── control        # 包控制文件（描述依赖、优先级等）
├── copyright      # 版权和许可信息
├── rules          # 构建规则（调用 debhelper 工具）
├── compat         # debhelper 兼容性版本
├── README.Debian  # Debian 特定的 README
└── ...            # 其他模板文件（如 manpage、docs 等）
```

### 5.典型场景示例

#### 1. 原生 `Debian` 包（无上游源码）

```bash
mkdir termsg-1.0.0 #<package>-<version>
cd termsg-1.0.0/
dh_make --native -e nardoo@sohu.com

#或
mkdir xxx
cd xxx
dh_make --native -e nardoo@sohu.com -p termsg_1.0.0#<name>_<version>
```

#### 2. **基于上游源码的包**

```bash
# 假设上游源码包下载下来并解压为了myproject-1.0/目录
ls
myproject-1.0 termsg_1.0.0.orig.tar.xz
cd myproject-1.0/
dh_make -s -e nardoo@sohu.com -p termsg_1.0.0#上级目录下的同级目录需要相应的tar包，在 Debian 打包规范中，orig.tar.xz 是上游原始源码的归档，用于区分 “上游源码” 和 “打包过程中添加的 Debian 配置（如 debian/ 目录下的文件）”。工具通过对比 orig.tar.xz 和本地源码的差异，生成补丁文件，确保打包过程的可追溯性。

#或者
Could not find termsg_1.0.0.orig.tar.xz
Either specify an alternate file to use with -f,
or add --createorig to create one.

#1.自动创建原始源码包（推荐，适合首次打包）
如果源码是本地开发的（没有单独的上游源码包），可以使用 --createorig 选项让工具自动生成 orig.tar.xz：
dh_make -s -e nardoo@sohu.com -p termsg_1.0.0 --createorig
#2.手动指定已有的原始源码包（适合有官方上游包的情况）
如果已经有官方发布的上游源码包（比如从项目官网下载的 termsg-1.0.0.tar.xz），只需将其重命名为规范格式，并通过 -f 选项指定:
# 先重命名（确保版本号与 control 文件一致，这里是 1.0.0）
mv termsg-1.0.0.tar.xz termsg_1.0.0.orig.tar.xz
dh_make -s -e nardoo@sohu.com -p termsg_1.0.0 -f termsg_1.0.0.orig.tar.xz
```

#### 3. **Python 项目**

```bash
dh_make --python -e nardoo@sohu.com
```

#### 4. **CMake 项目**

```bash
dh_make --cmake -e nardoo@sohu.com
```

### 6.注意事项

#### 1.版本一致性

`dh_make` 会从目录名（**为执行该命令的当前目录**）自动提取版本号（如 `myproject-1.0` → 版本 `1.0`），确保目录名格式正确。
否则请使用-p选项。

```bash
linux@linux-virtual-machine:~/dpkg$ dh_make  --native

For dh_make to find the package name and version, the current directory
needs to be in the format of <package>-<version>.  Alternatively use the
-p flag using the format <name>_<version> to override it.
The directory name you have specified is invalid!

Your current directory is:
/home/linux/dpkg
Perhaps you could try going to directory where the sources are?

Please note that this change is necessary ONLY during the initial
Debianization with dh_make.  When building the package, dpkg-source
will gracefully handle almost any upstream tarball.
#tarball：是 Linux 中对归档压缩包的常用称呼，通常以 .tar.gz .tar.bz2 等为后缀，包含软件的完整原始源码。

#dpkg-source 的兼容性强。开发者在打包时，不用特意对官方发布的源码压缩包做复杂的格式或结构改造 —— 不管是常见的 .tar.gz 格式，还是其他主流归档压缩格式的上游源码包，dpkg-source 都能自动适配，完成解压、校验、结合补丁生成符合 Debian 规范的源码包等操作，大幅降低开发者的打包适配成本。
```

#### 2.手动调整

生成的文件是模板，需要根据项目实际情况手动编辑（如调整 `control` 中的依赖）。

#### 3.与 `debhelper` 配合

`dh_make生成的`rules文件依赖`debhelper`需要确保系统已安装

```bash
 sudo apt install debhelper
```

------

## 2.`dpkg-buildpackage`命令

### 1. 签名相关选项

- **`-uc`**（`--unsigned-changes`）
  不使用`GPG`签名生成的 `.changes` 文件（记录构建过程和包信息的文件）。
- **`-us`**（`--unsigned-source`）
  不使用 `GPG`签名源代码包（`.dsc` 文件）。
  **适用场景**：本地开发或测试环境，无需正式发布时。

### 2. 权限模拟选项

- **`-rfakeroot`**
  使用 `fakeroot` 工具模拟 root 权限执行构建过程。
  **作用**：避免真正以 root 身份构建（安全风险），但允许在构建过程中执行需要 root 权限的操作（如创建文件系统层次结构）。
  **依赖**：需预先安装 `fakeroot` 包。

### 3. 架构指定选项

- `-aarmhf`

  指定构建的目标架构为

  ```shell
  armhf #（ARM 32 位硬浮点架构，常见于树莓派等设备）。
  ```

  影响：

  - 编译时使用与 `armhf` 兼容的工具链（如 `arm-linux-gnueabihf-gcc`）；
  - 生成的 `.deb` 包的 `Architecture` 字段标记为 `armhf`，只能安装在对应架构的系统上。

### 4. 编译优化选项

- `-nc`（--no-clean）

  构建前不清理源代码树（即不执行make clean）。

  **适用场景**：

  - 增量构建（节省时间）；
  - 调试构建问题（保留中间文件）。

### 5. 输出控制选项

- **`--changes-option=-quiet`**
  传递 `-quiet` 选项给底层的 `dpkg-genchanges` 工具，减少构建过程中的日志输出（仅显示错误和关键信息）。
  **效果**：使构建日志更简洁，适合自动化环境或批量构建。

### 6. 生成特定文件

1. **构建二进制包（不签名）**

   ```bash
   dpkg-buildpackage -us -uc -b#这会在父目录生成 .deb文件。
   ```

2. **为` ARM64 `架构交叉编译**

   ```bash
   dpkg-buildpackage -us -uc -b -aarm64
   ```

3. **生成源代码包**

   ```bash
   dpkg-buildpackage -S  #这会生成.dsc、.tar.gz 和 .diff.gz 文件。
   ```

4. **自动创建原始源代码 tarball**

   ```bash
   dpkg-buildpackage -us -uc --createorig
   ```

5. **静默构建（减少输出）**

   ```bash
   dpkg-buildpackage -us -uc -b --changes-option=-q
   ```

### 7.命令示例

为 `ARMhf `架构构建一个 `.deb` 软件包，不进行签名，使用模拟 root 权限，保留现有编译结果，并且输出简洁的构建日志。

```bash
dpkg-buildpackage -uc -us -rfakeroot -aarmhf -nc --changes-option=-quiet
```

## 3.相关命令常见问题

1. **依赖错误**
   确保 `debian/control` 中的 `Build-Depends` 包含所有必要的开发包。
2. **缺少 orig.tar.xz**
   使用 `--createorig` 自动创建，或手动提供上游源代码包。且格式需为xxx.orig.tar.xz
3. **架构不匹配**
   使用 `-a<架构>` 明确指定目标架构，或在 `control` 中设置 `Architecture: all`。
4. **签名失败**
   检查 GPG 密钥是否正确配置，或使用 `-us -uc` 跳过签名。

通过 `dpkg-buildpackage`，开发者可以高效地将软件打包为 `.deb` 格式，便于在 Debian/Ubuntu 系统上分发和安装。

------

# `debian`目录

`debian/`目录是 Debian、Ubuntu 等 Debian 系 Linux 发行版中构建 deb 软件包的核心配置目录，所有与打包相关的规则、依赖、元信息等均集中在此。打包工具（如`dpkg-buildpackage`、`debuild`）会通过读取该目录下的文件，完成从源码到 deb 二进制包的编译、打包流程。           

## 1.获取方式

`debian/`目录的获取主要有三种方式，优先推荐工具生成（标准化），手动创建适合特殊定制场景。

### 1.工具自动生成（推荐）

借助`dh-make`工具可快速生成符合`Debian`打包规范的`debian/`目录，这是最常用的方式，适合绝大多数打包场景。步骤如下：

1. 准备源码：将软件源码解压到单独目录，进入该目录（以之前的`termsg`项目为例）

   ```bash
   tar -xf termsg_1.0.0.tar.xz
   cd termsg-1.0.0
   ```

2. 生成 `debian` 目录：执行`dh-make`，工具会交互式生成配置，默认生成全套标准文件。`dh_make`执行后会提示选择软件包类型（如单二进制包、多二进制包等），新手直接回车选择默认的 “single binary” 即可，最终在当前目录生成`debian/`文件夹。

### 2.获取上游源码包

许多开源软件的官方源码包（或 Debian/Ubuntu 官方维护的源码包）中已包含成熟的`debian/`目录，可直接下载并复用：

- **从 Debian/Ubuntu 官方源获取源码包**

  用`apt source`命令下载某软件的完整源码包（包含`debian/`目录），例如获取`nginx`的源码包：

  ```bash
  # 安装源码获取工具（若未安装）
  sudo apt install dpkg-dev
  # 下载nginx的源码包（会自动拉取含debian目录的完整源码）
  apt source nginx
  ```

  执行后会在当前目录生成类似`nginx-1.21.6/`的文件夹，其中包含完整的`debian/`目录（官方已配置好打包规则）。

- **从软件上游仓库下载**

  部分软件的官方 Git 仓库（如 GitHub、GitLab）会在源码中附带`debian/`目录（用于官方打包），直接克隆仓库即可：

  ```bash
  git clone https://github.com/xxx/xxx.git  # 替换为目标软件仓库地址
  cd xxx
  ls debian/  # 查看是否存在debian目录
  ```

### 3. 手动创建（进阶场景）

若需高度自定义打包逻辑，可手动创建`debian/`目录及内部文件。这种方式需熟悉 Debian 打包规范，否则易导致打包失败。步骤如下：

1. 创建目录：在源码根目录执行`mkdir debian`
2. 手动创建核心文件：根据需求逐步创建`control`、`rules`等必需文件，后续会详细说明各文件的内容规范。

## 2.目录内容

下列以`dh_make`生成的`debian`目录为例进行简要介绍：

```bash
zhangg@ubuntu:~/myapp-1.0.0$ tree debian/
debian/
├── changelog#记录软件版本变更历史，供 dpkg 和用户查看
├── compat   #指定debhelper兼容性版本（如 13），影响构建工具行为
├── control  #定义包的元数据（名称、描述、依赖等)
├── copyright #声明软件版权信息和许可证
├── init.d.ex #SysV 风格的服务启动脚本模板（用于 /etc/init.d/）,若应用为服务，需删除 .ex 后缀并修改启动 / 停止逻辑。
├── manpage.1.ex #手册页（man page）模板
├── manpage.sgml.ex#手册页（man page）模板
├── manpage.xml.ex#手册页（man page）模板
├── menu.ex#桌面环境菜单配置模板（用于 /usr/share/menu/）。
├── myapp.cron.d.ex #定时任务配置模板（用于 /etc/cron.d/）。
├── myapp.default.ex #服务配置文件模板（用于 /etc/default/）。
├── myapp.doc-base.EX#文档索引配置（用于 /usr/share/doc-base/）
├── myapp-docs.docs
├── postinst.ex #安装 / 卸载前后执行的脚本。
├── postrm.ex#安装 / 卸载前后执行的脚本。
├── preinst.ex#安装 / 卸载前后执行的脚本。
├── prerm.ex#安装 / 卸载前后执行的脚本。
├── README.Debian
├── README.source
├── rules #控制编译和安装过程的 Makefile 脚本
├── source
│   └── format#声明包的源代码格式（通常为 3.0 (quilt)），无需修改
└── watch.ex#监控上游版本更新的配置（用于 uscan 工具）。 #基于上游包构建特有的
1 directory, 22 files
```

## 3.必要文件

`debian/`目录包含多个文件，其中部分是打包的核心必需文件，缺少或配置错误会直接导致打包失败；另有部分可选文件用于扩展打包功能。以下重点说明必需文件及常用扩展文件。具体参考[第 4 章 debian 目录中的必需内容](https://www.debian.org/doc/manuals/maint-guide/dreq.zh-cn.html)。

### 1.`control`文件

#### 1.作用介绍

主要描述软件包的元数据如名称（Package），版本（Version），描述（Description）等，是deb包必须具备的描述性文件，以便于软件的安装管理和索引。这个文件包含的很多供 **dpkg**、**dselect**、**apt-get**、**apt-cache**、**aptitude** 等包管理工具进行管理时所使用的许多变量均在 [Debian Policy Manual, 5 "Control files and their fields"](http://www.debian.org/doc/debian-policy/ch-controlfields.html) 中被定义。下面的 `control` 文件是 **dh_make** 命令为我们创建的：

#### 2.一键生成

control文件非首次编包，一般都会存在，若没有该文件可以通过下面命令进行生成：

```shell
dh_make -e 邮箱地址   -f  ../xxx.tar.gz
```

#### 3.具体字段介绍

[Ubuntu系统下deb包中control文件详解_deb control-CSDN博客](https://blog.csdn.net/Luckiers/article/details/118277548)
	   [Debian Policy Manual, 5 "Control files and their fields"](http://www.debian.org/doc/debian-policy/ch-controlfields.html) 

```shell
Source: termsg #源代码包的名称，如果软件包名称有两个词，用一个连字符（-）把它们连起来。软件包的名称只能有小写的英文字母，数字以及"+"和"-"

Section: application #该源码包要进入发行版本的分类，即列出软件属于的类别，可能的值包括admin（管理），games（游戏），gnome，kde，mail（电子邮件），misc（杂项）<注：misc是miscellaneous的简称>，net（网络），sound（声音），text（文本），utils（实用工具）和web（万维网），main（自由软件）、non-free（非自由软件）、contrib（依赖非自由软件的自由软件）。

Priority: extra  #描述用户安装此软件包的优先级，optional 优先级适用于与优先级为 required、important 或 standard 的软件包不冲突的新软件包。extra 优先级适用于与其他非 extra 优先级软件包冲突的新软件包。

Maintainer: users <users@sohu.com> # 维护者的姓名和邮箱

Build-Depends: debhelper (>= 7), autotools-dev #编译依赖，可以用dpkg-depcheck -d ./configure查看

Standards-Version: 3.7.3 #此软件包的标准版本号
Homepage: <insert the upstream URL, if relevant>

Package: termsg #二进制的包名
Section: application
Architecture: any #软件包架构，标明该软件包适用于何种cpu架构，all表明不需要根据架构做区分，编译生成一个软件包即可；any表示四种cpu架构需要分别编译会生成四个软件包；amd64，i386，armhf，arm64表示适用于该种cpu架构
Depends: ${shlibs:Depends}, ${misc:Depends} #依赖的软件包，当这些软件包都安装好后这个软件包才可安装
Pre-Depends：#依赖性强于Depends项中的软件包，必须正确配置后才可安装，慎用
Suggests：#建议安装的软件包
Conflicts：#冲突的软件包，当这些软件包卸载完后才可安装
Breaks：#安装本软件包后会损坏的软件包
Replaces：#替换的软件包
Description: package of terminal #二进制包的描述
 The packages is pub dbmanage meterpro pppd comm measure dataproc taskmetering gather nproparse menu sysconfig softroute.
```

### 2.`copyright`文件

这个文件包含了上游软件的版权以及许可证信息，软件版权和许可证信息。

### 3.`changelog`文件

这是一个**必须的**文件，用以解析版本号信息、适用的发行版和紧急程度。详细描述所做出的更改也是很好且很重要的。它将帮助下载软件包的人了解这个软件包中是否有他们需要知道的事情。它会被作为 `/usr/share/doc/termsg/changelog.Debian.gz` 保存在二进制包中。**dh_make** 创建了一个默认的文件，这是它的容貌：

```
1  termsg (1.0.0) unstable; urgency=medium
2
3   * Initial release.
4
5  -- linux <your.email@example.com>  Mon, 03 Nov 2025 09:26:37 +0800
6
```

第 1 行是软件包名、版本号、发行版和紧急程度。软件包名必须与实际的源代码包名相同，发行版应该是 `unstable`。除非有特殊原因，紧急程度默认设置为 medium（中等）。

第 3-5 行是一个很长的条目，记录了你在这个 Debian 修订版本中做出的修改(非上游修改——上游修改由上游作者创建并由另外一个文件维护，它们应被安装为 `/usr/share/doc/gentoo/changelog.gz`)。假设你的 ITP (Intent To Package，计划打包)的 Bug 号为 `12345`。新行必须插入在上一个以星号 `*` 开头的行的正下方。为了阻止软件包在打包完成之前被意外上传，将发行版值改成一个不可用的 `UNRELEASED` 将是一个很好的选择。最后它会成为以下的样子：

```
1  termsg (1.0.0) unstable; urgency=medium
2
3   * Initial release.
4   * This is my first Debian package.
5   * Adjusted the Makefile to fix $(DESTDIR) problems.
6
7  -- linux <your.email@example.com>  Mon, 03 Nov 2025 09:26:37 +0800
```

### 4.`rules`文件

#### 1.特征介绍

现在我们需要看看 dpkg-buildpackage(1) 用于实际创建软件包的 rules 文件。这个文件事实上是另一个 `Makefile`，但不同于上游源代码中的那个。和 `debian` 目录中的其他文件不同，这个文件被标记为可执行。强烈推荐参考链接：[系列二 详解 debian/rules | DeepinWiki](https://wiki.deepin.org/zh/待分类/02_按软件功能划分/02_开发人员常用软件介绍/01_编程开发/打包工具/相关内容/Debian发行版基础系列/详解debian_rules)

#### 2.`rules`文件中的 Target

每一个 `rules` 文件， 就像其他的 `Makefile` 一样，包含着若干 rules，其中每一个都定义了一个 target 以及其具体 操作。一个新的 rule 以自己的 target 声明(置于第一列)来起头。 后续的行都以 TAB 字符 (ASCII 9) 来开头，以指示 target 的具体行为。 空行和以井号 `#` 开头的行会被当作注释而被忽略。当想要执行一个 rule 的时候，就将 target（目标）名称作为命令行参数来调用。比如说， `debian/rules build `以及 `fakeroot make -f debian/rules binary` 会分别执行 `build`和 `binary`两个 target。

以下是对各 target 的简单解释：

|      Target       |                          执行的操作                          |
| :---------------: | :----------------------------------------------------------: |
|      `clean`      |      清理所有编译的、生成的或编译树中无用的文件。(必须)      |
|      `build`      |     在编译树中将代码编译为程序并生成格式化的文档。(必须)     |
|   `build-arch`    |      在编译树中将代码编译为依赖于体系结构的程序。(必须)      |
|   `build-indep`   |     在编译树中将代码编译为独立于平台的格式化文档。(必须)     |
|     `install`     | 把文件安装到 `debian`目录内为各个二进制包构建的文件树。如果有定义，那么 `binary` target 则会依赖于此 target。(可选) |
|     `binary`      | 创建所有二进制包(是 `binary-arch` 和 `binary-indep` 的合并)。(必须) |
|   `binary-arch`   | 在父目录中创建平台依赖(`Architecture: any`)的二进制包。(必须) |
|  `binary-indep`   | 在父目录中创建平台独立(`Architecture: all`)的二进制包。(必须) |
| `get-orig-source` |           从上游站点获得最新的原始源代码包。(可选)           |

#### 3.默认的 `rules` 文件

新版本的 **dh_make** 会生成一个使用 **dh** 命令的非常简单但非常强大的默认的 `rules` 文件：

```makefile
 1 #!/usr/bin/make -f
 2 # See debhelper(7) (uncomment to enable)
 3 # output every command that modifies files on the build system.
 4 #DH_VERBOSE = 1
 5 
 6 # see FEATURE AREAS in dpkg-buildflags(1)
 7 #export DEB_BUILD_MAINT_OPTIONS = hardening=+all
 8
 9 # see ENVIRONMENT in dpkg-buildflags(1)
10 # package maintainers to append CFLAGS
11 #export DEB_CFLAGS_MAINT_APPEND  = -Wall -pedantic
12 # package maintainers to append LDFLAGS
13 #export DEB_LDFLAGS_MAINT_APPEND = -Wl,--as-needed
14 
15
16 %:
17         dh $@ 

#(注：该文件添加了行号并删去了一些注释。实际的 `rules` 文件里开头的空格是 TAB 填充的。)
```

1. **DH_VERBOSE**
   可以取消第 4 行的注释，以设置 `DH_VERBOSE` 变量为 1， 于是 **dh** 命令就会输出它将要使用的 **dh_\*** 命令。 也可以在此添加一行 `export DH_OPTIONS=-v` ， 于是 **dh_\*** 命令 同样也会输出它正在调用的命令。 这能帮助理解在这个简单的 `rules` 文件背后发生了什么，以及帮助进行调试。新的 **dh** 被设计来作为 `debhelper` 工具的核心部分，并不隐藏任何东西。
2. **pattern rule**
   第 16 和 17 行使用了模式匹配即正则表达式 ，以此隐式地完成所有工作。 其中的百分号意味着“任何 targets”， 它会以 target 名称作参数调用单个程序 **dh**。  **dh** 命令是一个包装脚本，它会根据参数执行妥当的 **dh_\*** 程序序列。比如：

- `debian/rules clean` 运行了 `dh clean`，接下来实际执行的命令为：

  ```
  dh_testdir
  dh_auto_clean
  dh_clean
  ```

- `debian/rules build` 运行了 `dh build`，其实际执行的命令为：

  ```
  dh_testdir
  dh_auto_configure
  dh_auto_build
  dh_auto_test
  ```

- `fakeroot debian/rules binary` 执行了 `fakeroot dh binary`，其实际执行的命令为：

  ```
  dh_testroot
  dh_prep
  dh_installdirs
  dh_auto_install
  dh_install
  dh_installdocs
  dh_installchangelogs
  dh_installexamples
  dh_installman
  dh_installcatalogs
  dh_installcron
  dh_installdebconf
  dh_installemacsen
  dh_installifupdown
  dh_installinfo
  dh_installinit
  dh_installmenu
  dh_installmime
  dh_installmodules
  dh_installlogcheck
  dh_installlogrotate
  dh_installpam
  dh_installppp
  dh_installudev
  dh_installwm
  dh_installxfonts
  dh_bugfiles
  dh_lintian
  dh_gconf
  dh_icons
  dh_perl
  dh_usrlocal
  dh_link
  dh_compress
  dh_fixperms
  dh_strip
  dh_makeshlibs
  dh_shlibdeps
  dh_installdeb
  dh_gencontrol
  dh_md5sums
  dh_builddeb
  ```

- `fakeroot debian/rules binary-arch` 执行了 `fakeroot dh binary-arch`，其效果等同于 `fakeroot dh binary` 并附加 `-a` 参数于每个命令后。

- `fakeroot debian/rules binary-indep` 执行了 `fakeroot dh binary-indep`，这会运行几乎和 `fakeroot dh binary` 一样的命令，但 **dh_strip**、**dh_makeshlibs** 和 **dh_shlibdeps** 除外，其他命令则均附加`-i` 选项。

#### 4.`dh_*`的等价命令

**dh_\*** 命令的功能依其名称不言而喻。 不过其中有一些值得在这里进行简要解释， 假定有一个基于 `Makefile` 的典型构建环境： 

- **dh_auto_clean** 通常在 `Makefile` 存在且有 `distclean` target 时执行以下命令:

  ```makefile
  make distclean
  ```

- **dh_auto_configure** 在 `./configure` 存在时通常执行以下命令(省略部分参数以方便此处阅读)。

  ```makefile
  ./configure --prefix=/usr --sysconfdir=/etc --localstatedir=/var ...
  ```

- **dh_auto_build** 通常使用以下命令执行 `Makefile` 中的第一个 target。

  ```makefile
  make
  ```

- **dh_auto_test** 通常在 `Makefile` 存在且有 `test` target 时执行以下命令。

  ```makefile
  make test
  ```

- **dh_auto_install** 通常在 `Makefile` 存在且有 `install` target 时执行以下命令(进行了换行以便阅读)。

  ```makefile
  make install \
    DESTDIR=/path/to/package_version-revision/debian/package
  ```

所有需要 **fakeroot** 命令的都包含了 **dh_testroot**。如果没有使用 fakeroot，那将会报错并退出。关于 **dh_make** 生成的 `rules` 文件，应该知道的最重要的事是，它仅仅是一个建议。它对多数简单的软件包有效，但对于更复杂的则要大胆对其进行定制以满足需要。

------

## 4.其他文件

### 1.`compat`文件

该文件定义了`debhelper`的兼容级别。目前应当使用如下方法将其设置为`debhelper` V10：

```shell
$ echo 7 > debian/compat #Build-Depends: debhelper (>= 7), autotools-dev
```

在特定场景下，可以在需要兼容旧版本系统时使用兼容等级9。然而，不建议使用任何低于 9 的兼容等级，在新建软件包时也应避免使用这些低的等级。

## 3.修改配置

1. **删除 `.ex` 或 `.EX` 后缀**：
   对需要使用的文件，移除扩展名（如 `init.d.ex` → `init.d`）。

2. **修改关键文件**：
   重点修改 `control`、`changelog`、`copyright`、`rules` 和脚本钩子（如 `postinst`）。

3. **忽略不需要的文件**：
   不需要的文件（如 `menu.ex`）可以直接删除。

# 6.高级功能

1. **并行构建**
   使用 `-j` 选项指定并行编译的线程数：

   ```bash
   dpkg-buildpackage -j$(nproc)  # 使用所有 CPU 核心
   ```

2. **自定义构建环境**
   通过环境变量配置编译器和工具链：

   ```bash
   CC=arm-linux-gnueabihf-gcc dpkg-buildpackage -aarmhf
   ```

3. **使用 pbuilder 进行隔离构建**
   在干净的 chroot 环境中构建，确保依赖纯净：

   ```bash
   pbuilder build ../myproject_1.0.dsc
   ```

# 7.相关工具链

1. **`debhelper`**
   提供 `dh_*` 系列命令（如`dh_make` `dh_auto_configure`、`dh_install`），简化 `rules` 文件编写。
2. **`dpkg-deb`**
   直接操作 `.deb` 包的工具（创建、提取、查看）。
3. **`debuild`**
   基于 `dpkg-buildpackage` 的更高级前端，提供额外功能（如 pbuilder 集成）。
4. **`lintian`**
   检查包是否符合 Debian 政策，发现潜在问题。

# 8.原生包和非原生包

- **原生包**：就像 “本地特产”。
  比如某个小吃（比如北京的豆汁儿），从原料到制作方法，都是在本地（Debian 系统）原创的，只适合在本地环境（Debian/Ubuntu 等基于 Debian 的系统）里 “吃”（安装使用），没有其他 “老家”（其他系统）。
  对应的软件包：比如 Debian 系统自己开发的一些工具（如`dpkg`本身），它们的源代码和打包方式都是 Debian 独有的，只存在于 Debian 的软件仓库里。
- **非原生包**：就像 “进口商品”。
  比如国外的零食（比如日本的薯片），它的 “老家”（源代码）在国外（比如最初是为 Red Hat 系统开发的，或者是跨平台的通用软件），后来被 “进口” 到本地（Debian 系统），并按照本地的 “包装标准”（Debian 的打包规则）重新包装，才能在本地 “销售”（安装）。
  对应的软件包：比如`Firefox`浏览器、`Python`解释器，它们的源代码是通用的，最初可能不是为 Debian 开发的，但被 Debian 开发者重新打包成符合 Debian 格式的包，放进了 Debian 的仓库里。

简单说，核心区别就是：**原生包是 Debian “土生土长” 的，非原生包是 “外来户” 被 Debian 重新包装的**。

# 

# 项目目录

## 1.目录结构

一个典型的 `Debian` 包项目结构如下：

```bash
myproject/
├── debian/# Debian 包控制文件目录
│   ├── 【changelog】#用于记录软件包的版本变更历史，包括每个版本的更新内容、发布时间、维护者信息等。它不仅是给用户看的 “更新日志”，也是 dpkg、debhelper 等工具识别版本号、构建包的关键依据。
|	├── compat
|	├── control
|	├── copyright
|	├── init.d.ex#让系统通过 service myapp start/stop/restart 等命令控制 myapp 服务。				它是 /etc/init.d/ 目录下的服务控制脚本，用于定义 myapp 如何启动、停止、				  重启等。
|	├── manpage.1.ex#手册页是 Linux 系统中标准的帮助文档（通过 man 程序名 查看），这个示					例文件提供了手册页的基本格式，用于说明 myapp 的功能、用法和参数。
|	├── manpage.sgml.ex
|	├── manpage.xml.ex#生成的手册页示例文件，作用和 manpage.1.ex 一致 —— 都是为了生成程序					 的手册页（manpage），只是编写格式不同。
|	├── menu.ex#这个文件用于在 Linux 桌面环境的应用菜单中添加 myapp 的启动项（比如在 “应用				程序” 菜单中显示一个可点击的图标或条目）
|	├── myapp.cron.d.ex#作用是定义软件包 myapp 需要的定时任务。
|	├── myapp.default.ex#集中管理服务的启动参数（如 DAEMON_OPTS="-v --config 							 /etc/myapp.conf"），避免硬编码到 init.d 脚本中。用户可通过修改 					  /etc/default/myapp 自定义服务启动选项，无需改 init.d 脚本。
|	├── myapp.doc-base.EX
|	├── myapp-docs.docs #app文档说明相关
|	├── postinst.ex#创建 myapp 用户，启动服务（systemctl start myapp），并设置开机自启		（systemctl enable myapp）。
|	├── postrm.ex#删除 myapp 用户，清理 /var/run/myapp 临时目录，移除自启配置				（systemctl disable myapp）。
|	├── preinst.ex#检查是否已安装依赖的 libxxx 库，若未安装则报错阻止安装
|	├── prerm.ex#停止正在运行的 myapp 服务（systemctl stop myapp），避免文件被占用。
		#用于在软件包的安装、升级、卸载过程中自动执行特定操作（比如配置服务、创建用户、清理		  文件等）。这四个脚本按执行顺序和场景分工，核心作用是 “衔接软件包生命周期的关键节		  点，确保安装 / 卸载过程平滑且符合系统规范”。
|	├── README.Debian#文件是打包者与用户的 “沟通桥梁”，用于补充 Debian 系统下的软件使用细节，让用户更方便地使用包。如果不需要可以删除。
|	├── README.source#这个文件用于描述源代码包的信息（比如源代码的获取方式、补丁的应用说						明、特殊构建流程等），遵循 Debian 政策手册第 4.14节的规范。如果不需					要可以删除。
|	├── rules
|	├── source
│   |	└── format
|	|── watch.ex#用于配合 uscan 工具监控上游（upstream）软件的新版本更新。简单说，它的作用				是：自动检查软件（比如 myapp）的上游开发者是否发布了新版本，方便及时更新 			   Debian 包。
│   ├── install              # 文件安装位置说明
│   └── [其他可选文件]        # 如triggers, conffiles等
├── src/                     # 源代码目录（可选）
└── [其他项目文件]
```

## 2.目录格式

### 1.默认情况

`dh_make`设计上默认通过**当前目录的名称**推断包名（package name）和版本号（version），因此要求项目目录名必须遵循格式：

```shell
<package-name>-<version>
#例如：myapp-1.0.0、utils-2.3
#如果项目目录名为myapp-1.0.0，执行dh_make命令时会自动将包名设为myapp，版本设为1.0.0，无需额外参数。
```

- **包名（package name）**
  目录名中 `-` 前面的部分（如 `myapp`）
- **版本号（version）**
  目录名中 `-` 后面的部分（如 `1.0.0`）

如果目录名不符合这个格式（比如 `myapp_v1.0`、`test`、`1.0-myapp` 等），在直接执行`dh_make` 命令时就无法识别包名和版本号，从而抛出 **目录名无效** 的错误。

```shell
For dh_make to find the package name and version, the current directory
needs to be in the format of <package>-<version>.  Alternatively use the
_-p flag using the format <name>_<version> to override it.
The directory name you have specified is invalid!

Your current directory is:
/home/zhangg/dpkg
Perhaps you could try going to directory where the sources are?

Please note that this change is necessary ONLY during the initial
Debianization with dh_make.  When building the package, dpkg-source
will gracefully handle almost any upstream tarball.
```

### 2.带参数自定义

根据前面的出错提示也可以看到，也可以使用 `-p` 标志并按照 `<名称>_<版本>` 的格式来覆盖重命名它。如果不想重命名目录，可以通过 `-p` 选项手动指定包名和版本，格式为 `<package-name>_<version>`（注意这里用下划线 `_` 分隔，而非目录名中的 `-`）：

```bash
# 假设当前目录名为myproject，手动指定包名为myapp，版本为1.0.0
dh_make -s -y -e your_email@example.com -p myapp_1.0.0

#-p后面的参数格式是包名_版本号（必须用下划线），例如utils_2.3、tools-extra_0.9.1（包名允许包含 `-`，但版本号中通常不建议)。
```

## 3.安装、卸载脚本

通俗举例：以一个 `myapp` 服务为例

假设 `myapp` 是一个后台服务，这四个脚本的执行流程和内容可能如下：

1. **安装 `myapp` 时**：

   - `preinst`：检查是否已安装依赖的 `libxxx` 库，若未安装则报错阻止安装。
   - `postinst`：创建 `myapp` 用户，启动服务（`systemctl start myapp`），并设置开机自启（`systemctl enable myapp`）。

2. **卸载 `myapp` 时**：

   - `prerm`：停止正在运行的 `myapp` 服务（`systemctl stop myapp`），避免文件被占用。
   - `postrm`：删除 `myapp` 用户，清理 `/var/run/myapp` 临时目录，移除自启配置（`systemctl disable myapp`）。

3. **升级 `myapp` 到新版本时**：
   - 新版 `preinst`：备份旧配置文件（`/etc/myapp.conf` 到 `/etc/myapp.conf.bak`）。
   - 旧版 `prerm`：停止旧版本服务。
   - 新版 `postinst`：合并旧配置到新文件，启动新版本服务。
   - 旧版 `postrm`：清理旧版本残留的临时文件。

## 4.日志版本号编写

**推荐工具**：用 `dch` 命令自动生成 / 更新 `changelog`，避免手动写格式出错：

```bash
dch -i  # 增加一个 Debian 修订号（如 1.0.0-1 → 1.0.0-2）
dch -v 1.1.0-1  # 直接指定新版本号（如升级上游版本到 1.1.0）
```



## 5.自动生成的脚本

执行 `dh_make` 时会自动生成以下文件：

- `debian/changelog`（初始版本）
- `debian/control`（模板）
- `debian/copyright`（模板）
- `debian/rules`（基本规则）
- `debian/source/format`（包格式声明）

## 6.需要手动修改的文件

1. **`debian/control`**：添加正确的依赖和描述
2. **`debian/rules`**：定制编译和安装步骤
3. **`debian/changelog`**：更新版本和变更记录
4. **`debian/copyright`**：添加实际的版权信息
5. **`debian/install`**：指定文件安装位置

## 7.测试包

### 1. 安装包

```bash
sudo dpkg -i ../myproject_1.0-1_amd64.deb
```

### 2. 检查安装内容

```bash
dpkg -L myproject
```

### 3. 卸载包

```bash
sudo dpkg -r myproject
```

## 8.高级技巧

### 1. 添加 postinst 脚本

在 `debian/` 目录下创建 `postinst` 文件：

```bash
#!/bin/sh
set -e

case "$1" in
    configure)
        # 配置逻辑
        ;;
    abort-upgrade|abort-remove|abort-deconfigure)
        # 回滚逻辑
        ;;
esac

exit 0
```

并设置执行权限：

```bash
chmod +x debian/postinst
```

### 2. 添加 conffiles

在 `debian/` 目录下创建 `conffiles` 文件，列出配置文件：

```plaintext
/etc/myproject.conf
```

## 9.常见问题

1. **依赖错误**
   确保 `control` 文件中的 `Depends` 字段正确
2. **文件权限问题**
   使用 `dh_install` 或 `debian/install` 正确设置权限
3. **版本号冲突**
   每次构建前更新 `changelog` 中的版本号

通过以上步骤，可以为`Debian/Ubuntu` 系统创建完整的 `.deb` 安装包。实际操作中可能需要根据项目特性进行调整。



# 6.参考链接

[如何为Debian/Ubuntu制作deb安装包-谢先斌的博客](https://www.xiexianbin.cn/linux/ubuntu/dpkg-build/index.html#初始化-1)

https://www.debian.org/doc/manuals/maint-guide/

# 打包流程

## 1.准备源代码与环境

- **获取上游源代码**
  拿到原始的软件源代码（如 `.tar.gz` 压缩包），需符合 `Debian` 对 “未修改原始代码” 的要求（即后续会生成 `orig.tar.xz` 包）。

- **解压并进入目录**
  例如 `tar -zxvf myapp-1.0.tar.gz && cd myapp-1.0`。

- **安装必要工具**

  确保系统已安装打包工具链，如

  ```bash
  build-essential #（基础编译工具）
  
  debhelper #（辅助打包脚本）
  
  dh-make #（快速生成打包模板）
  ```

  ```bash
  sudo apt-get install build-essential debhelper dh-make
  ```

## 2.生成 `Debian `打包目录

- **创建`debian/`目录**
  项目根目录需包含 `debian/` 目录，其中包含控制文件（如 `control`、`rules`、`changelog` 等），因此需要在当前目录下建立`debian/`目录。

  可通过`dh-make`快速生成模板（根据软件类型选择`single`单二进制包或`library` 库等）：

  ```bash
  dh_make -s  # -s 表示单二进制包，会在当前目录生成 debian/ 模板
  ```

- **手动完善 `debian/` 内容**
  据项目特性及编译构建环境，修改控制文件。

  （核心文件）：

  - `debian/control`：声明包名、版本、依赖、描述等核心信息（必须修改）。
  - `debian/rules`：打包的核心规则（如编译命令、安装路径，通常基于 `debhelper` 语法）。
  - `debian/changelog`：记录版本更新日志（格式严格，需包含版本号、发布信息）。
  - `debian/copyright`：软件许可证信息（声明版权归属和授权方式）。
  - 其他可选文件：如 `debian/install`（指定文件安装路径）、`debian/postinst`（安装后执行的脚本）等。

## 3.验证并修改源代码（可选）

  - 如果需要对上游源代码做 `Debian` 特定修改（如适配 `Debian` 目录结构、修复兼容性问题），需在此步骤进行，并通过 `debian/patches/` 目录管理补丁（推荐用 `quilt` 工具）。
  - 确保修改不会破坏原始代码的完整性，后续构建时会通过 `orig.tar.xz` 区分原始代码与修改。

## 4. 生成原始代码包

- `Debian` 要求必须有一个 “未修改的原始源代码包”，命名格式为 `<package>_<version>.orig.tar.xz`（如 `myapp_1.0.0.orig.tar.xz`）。

- 若源代码是首次打包且未生成过该文件，可在构建时自动创建：

  ```bash
  dpkg-buildpackage 
  ```

## 5.执行构建命令

```bash
dpkg-buildpackage [选项]

-us：不签署源代码包
-uc：不签署二进制包
-b ：仅构建二进制包（不生成源代码包）
-B ：构建二进制包和依赖的架构相关包
--a<架构>：指定目标架构（如 -aarm64）
--createorig：自动创建原始源代码 tarball
```

- **构建过程中会自动完成**

  1. 检查构建依赖（若缺失会报错，需先安装）；
  2. 调用 `debian/rules` 中的规则编译源代码（如 `make`、`cmake` 等）；
  3. 将编译产物复制到临时目录（模拟安装路径）；
  4. 生成 `.deb` 包及相关辅助文件（如源码包 `.dsc`、变更日志 `.changes` 等）。

## 6.处理构建结果

构建完成后，会在父目录生成以下文件：

  ```bash
- .deb：二进制安装包
- .dsc：源代码包描述文件
- .changes：包变更记录
- .orig.tar.xz：原始源代码压缩包（若指定 `--createorig`）
  ```

## 7.测试与发布

- 本地测试安装

  用`dpkg -i`安装生成的`.deb`包，验证是否正常运行：

  ```bash
  sudo dpkg -i ../myapp_1.0.0-1_amd64.deb
  ```

  若安装后依赖缺失，可通过 `sudo apt -f install` 自动修复。

- **正式发布**（可选）
  若需分发，可使用 `GPG` 签名包（`dpkg-buildpackage` 不加 `-us -uc`），然后上传到 `Debian` 仓库或自己的源。



# 打包流程总结

## 1.上游包

1. 安装相关依赖
2. 

[Debian Sources | Debian Sources](https://sources.debian.org/)

```shell
 git clone https://salsa.debian.org/toolchain-team/elfutils.git#基于debian官网的软件包修改而来
```

## 2.原生包

1. 安装相关依赖

   ```shell
   sudo apt-get install build-essential binutils lintian debhelper dh-make devscripts
   ```

2. 生成`Debian/`目录

   ```shell
   dh_make --single --native -e your.email@example.com -p termsg_1.0.0 #纯原创
   ```

   

