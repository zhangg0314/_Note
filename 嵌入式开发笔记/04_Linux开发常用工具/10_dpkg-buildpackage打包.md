[如何为Debian/Ubuntu制作deb安装包-谢先斌的博客](https://www.xiexianbin.cn/linux/ubuntu/dpkg-build/index.html#初始化-1)

https://www.debian.org/doc/manuals/maint-guide/

# dpkg-buildpackage介绍

`dpkg-buildpackage` 是 `Debian/Ubuntu` 系统中用于构建软件包的核心工具，它会调用一系列底层工具（如 `dpkg-deb`、`dpkg-gencontrol`）来自动生成 `.deb` 软件包( 相当于 Windows 中的 **`.msi` 或 `.exe` 安装文件**)。以下是其详细介绍：

## 1.核心功能

1. **自动化构建流程**
   从源代码生成完整的 `.deb` 包，包括编译、打包、生成控制文件等步骤。
2. **多架构支持**
   支持交叉编译（如在` x86` 机器上为 ARM 架构构建包）。
3. **签名与验证**
   支持使用` GPG `对生成的包进行数字签名，确保完整性。
4. **依赖检查**
   自动检查并处理构建依赖（如所需的开发库）。

## 2.基本工作流程

1. **准备源代码**
   项目需包含 `debian/` 目录，其中包含控制文件（如 `control`、`rules`、`changelog` 等），因此需要在当前目录下建立`debian/`目录。

2. **修改控制文件**

   根据项目特性及编译构建环境，修改控制文件。

3. **执行构建命令**

   ```bash
   dpkg-buildpackage [选项]
   
   -us：不签署源代码包
   -uc：不签署二进制包
   -b ：仅构建二进制包（不生成源代码包）
   -B ：构建二进制包和依赖的架构相关包
   --a<架构>：指定目标架构（如 -aarm64）
   --createorig：自动创建原始源代码 tarball
   ```

4. **生成输出文件**
   构建完成后，会在父目录生成以下文件：

   - `.deb`：二进制安装包
   - `.dsc`：源代码包描述文件
   - `.changes`：包变更记录
   - `.orig.tar.xz`：原始源代码压缩包（若指定 `--createorig`）

## 3.关键配置文件

`dpkg-buildpackage` 依赖 `debian/` 目录下的多个配置文件：

1. **`control`**
   描述包的元数据（名称、版本、依赖、描述等）：

   ```plaintext
   Source: myproject
   Section: utils
   Priority: optional
   Maintainer: Your Name <your_email@example.com>
   Build-Depends: debhelper (>= 13), build-essential
   ...
   ```

2. **`rules`**
   控制编译和安装过程的 Makefile 脚本：

   ```makefile
   %:
       dh $@
   
   override_dh_auto_configure:
       ./configure --prefix=/usr
   ```

3. **`changelog`**
   记录包的版本变更历史：

   ```plaintext
   myproject (1.0-1) unstable; urgency=medium
   
     * Initial release.
   
    -- Your Name <your_email@example.com>  Mon, 1 Jan 2024 12:00:00 +0000
   ```

4. **`copyright`**
   软件版权和许可证信息。

5. **`install`**
   指定文件安装位置：

   ```plaintext
   bin/myproject usr/bin/
   ```

## 4.常用命令示例

1. **构建二进制包（不签名）**

   ```bash
   dpkg-buildpackage -us -uc -b
   ```

2. **为` ARM64 `架构交叉编译**

   ```bash
   dpkg-buildpackage -us -uc -b -aarm64
   ```

3. **生成源代码包**

   ```bash
   dpkg-buildpackage -S
   ```

4. **自动创建原始源代码 tarball**

   ```bash
   dpkg-buildpackage -us -uc --createorig
   ```

5. **静默构建（减少输出）**

   ```bash
   dpkg-buildpackage -us -uc -b --changes-option=-q
   ```

## 5.高级功能

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

## 6.相关工具链

1. **`debhelper`**
   提供 `dh_*` 系列命令（如 `dh_auto_configure`、`dh_install`），简化 `rules` 文件编写。
2. **`dpkg-deb`**
   直接操作 `.deb` 包的工具（创建、提取、查看）。
3. **`debuild`**
   基于 `dpkg-buildpackage` 的更高级前端，提供额外功能（如 pbuilder 集成）。
4. **`lintian`**
   检查包是否符合 Debian 政策，发现潜在问题。

## 7.常见问题及解决

1. **依赖错误**
   确保 `debian/control` 中的 `Build-Depends` 包含所有必要的开发包。
2. **缺少 orig.tar.xz**
   使用 `--createorig` 自动创建，或手动提供上游源代码包。
3. **架构不匹配**
   使用 `-a<架构>` 明确指定目标架构，或在 `control` 中设置 `Architecture: all`。
4. **签名失败**
   检查 GPG 密钥是否正确配置，或使用 `-us -uc` 跳过签名。

通过 `dpkg-buildpackage`，开发者可以高效地将软件打包为 `.deb` 格式，便于在 Debian/Ubuntu 系统上分发和安装。



# 安装依赖

```bash
apt install build-essential binutils lintian debhelper dh-make devscripts
```

# 项目目录

## 1.目录结构

一个典型的 `Debian` 包项目结构如下：

```plaintext
myproject/
├── debian/                  # Debian 包控制文件目录
│   ├── changelog            # 版本变更记录
│   ├── compat               # debhelper 兼容性版本
│   ├── control              # 包的核心元数据
│   ├── copyright            # 版权信息
│   ├── rules                # 编译和打包规则
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

如果目录名不符合这个格式（比如 `myapp_v1.0`、`test`、`1.0-myapp` 等），在执行`dh_make` 命令时就无法识别包名和版本号，从而抛出 **目录名无效** 的错误。

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

## 2.带参数自定义

根据前面的出错提示也可以看到，或者，使用 `-p` 标志并按照 `<名称>_<版本>` 的格式来覆盖重命名它。如果不想重命名目录，可以通过 `-p` 选项手动指定包名和版本，格式为 `<package-name>_<version>`（注意这里用下划线 `_` 分隔，而非目录名中的 `-`）：

```bash
# 假设当前目录名为myproject，手动指定包名为myapp，版本为1.0.0
dh_make -s -y -e your_email@example.com -p myapp_1.0.0

#-p后面的参数格式是包名_版本号（必须用下划线），例如utils_2.3、tools-extra_0.9.1（包名允许包含 `-`，但版本号中通常不建议)。
```



# 生成`Debian/`目录

## 1.命令介绍

`dh_make` 是 Debian 打包系统中的核心工具，用于**自动生成 `debian/` 目录的初始骨架**。它会根据项目类型（原生包、上游源码包等）创建必要的控制文件，大幅减少手动配置的工作量。

### 1.基本用法

```bash
dh_make [选项] [包版本]
```

**示例**：为项目创建 Debian 打包骨架

```bash
cd myproject/
dh_make --single --native -e your.email@example.com
```

### 2.核心选项详解

#### 1. **包类型选项**

| 选项           | 作用                                             |
| -------------- | ------------------------------------------------ |
| `--native`     | 创建原生 Debian 包（无上游源码，纯 Debian 项目） |
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

### 3.生成的关键文件

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

### 4.典型场景示例

#### 1. **原生 Debian 包（无上游源码）**

```bash
dh_make --native -e your.email@example.com
```

#### 2. **基于上游源码的包**

bash











```bash
# 假设上游源码在 myproject-1.0/ 目录
cd myproject-1.0/
dh_make -s -e your.email@example.com
```

#### 3. **Python 项目**

bash











```bash
dh_make --python -e your.email@example.com
```

#### 4. **CMake 项目**

bash











```bash
dh_make --cmake -e your.email@example.com
```

### 五、注意事项

1. **版本一致性**

   - `dh_make` 会从目录名自动提取版本号（如 `myproject-1.0` → 版本 `1.0`），确保目录名格式正确。

2. **手动调整**

   - 生成的文件是模板，需要根据项目实际情况手动编辑（如调整 `control` 中的依赖）。

3. **与 `debhelper` 配合**

   - ```
     dh_make
     ```

      

     生成的

      

     ```
     rules
     ```

      

     文件依赖

      

     ```
     debhelper
     ```

     ，确保系统已安装：

     bash

     

     

     

     

     

     ```bash
     sudo apt install debhelper
     ```

     

     

     

     ![img](data:image/png;base64,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)

通过 `dh_make`，你可以快速搭建 Debian 包的基础结构，然后专注于定制特定的打包规则和元数据。这大大降低了手动编写复杂控制文件的门槛。

在**项目根目录**下执行：

```bash
dh_make -s -y -e your_email@example.com #这会自动生成 debian 目录及基础文件。
```

### 2.出错情况

```shell
 dh_make -s -y -e your_email@example.com -p sg698_1.0.0
```

```shell
Could not find sg698_1.0.0.orig.tar.xz
Either specify an alternate file to use with -f,
or add --createorig to create one.
```



### 3.原始源代码包获取

#### 1.源码格式要求

`Debian`打包工具要求项目存在一个**未修改的原始源代码压缩包**（称为 `orig.tar.xz` 或 `orig.tar.gz`），用于区分上游（upstream）源代码和 Debian 打包的修改（如 `debian/` 目录中的配置）。其命名格式为：
`<package>_<version>.orig.tar.<压缩格式>`
例如：`myapp_1.0.0.orig.tar.xz`。当运行 `dh_make` 或 `dpkg-buildpackage` 时，工具会自动查找这个文件，若找不到，会提示你提供它（用 `-f`），或自动创建它（用 `--createorig`）。

#### 2.解决方法

根据提示，有两种常用解决方案：

##### 1.自动创建原始源代码包（推荐，适用于本地项目）

如果项目是**自己开发的**（非上游已发布的软件），可以用 `--createorig` 让工具自动生成 `orig.tar.xz`：

```bash
# 在项目目录（如 myapp-1.0.0）中运行
dh_make -s -y -e your_email@example.com --createorig
```

- `--createorig` 会将当前目录的所有文件（除 `debian/` 目录，若已存在）打包为 `../myapp_1.0.0.orig.tar.xz`（生成在父目录）。
- 后续构建时，工具会识别这个自动创建的原始包，不再报错。
- ```bash
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
  └── watch.ex#监控上游版本更新的配置（用于 uscan 工具）。
  1 directory, 22 files
  ```

##### 2.手动指定已有的原始源代码包（适用于上游已有发布包）

如果项目是**基于上游已发布的源代码**（例如从官网下载的 `myapp-1.0.0.tar.gz`），可以用 `-f` 手动指定该文件：

```bash
# 假设上游包位于父目录，名为 myapp-1.0.0.tar.gz
dh_make -s -y -e your_email@example.com -f ../myapp-1.0.0.tar.gz

#工具会自动将该文件重命名为 `myapp_1.0.0.orig.tar.gz` 并使用（注意：文件名需符合 `package_version.orig.tar.xxx` 格式）。
```

## 4.修改配置

1. **删除 `.ex` 或 `.EX` 后缀**：
   对需要使用的文件，移除扩展名（如 `init.d.ex` → `init.d`）。

2. **修改关键文件**：
   重点修改 `control`、`changelog`、`copyright`、`rules` 和脚本钩子（如 `postinst`）。

3. **忽略不需要的文件**：
   不需要的文件（如 `menu.ex`）可以直接删除。

## 5.构建包

### 1. 清理和检查

```bash
dpkg-buildpackage -us -uc -nc
```

- `-us`：不签署源代码包
- `-uc`：不签署变更日志
- `-nc`：不清理源文件（加快构建）

### 2. 生成二进制包

```bash
dpkg-buildpackage -b #这会在父目录生成 `.deb` 文件。
```

### 3. 生成完整源码包

```bash
dpkg-buildpackage -S #这会生成 `.dsc`、`.tar.gz` 和 `.diff.gz` 文件。
```

## 6.自动生成的脚本

执行 `dh_make` 时会自动生成以下文件：

- `debian/changelog`（初始版本）
- `debian/control`（模板）
- `debian/copyright`（模板）
- `debian/rules`（基本规则）
- `debian/source/format`（包格式声明）

## 7.需要手动修改的文件

1. **`debian/control`**：添加正确的依赖和描述
2. **`debian/rules`**：定制编译和安装步骤
3. **`debian/changelog`**：更新版本和变更记录
4. **`debian/copyright`**：添加实际的版权信息
5. **`debian/install`**：指定文件安装位置

## 8.测试包

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

## 9.高级技巧

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

## 10.常见问题

1. **依赖错误**
   确保 `control` 文件中的 `Depends` 字段正确
2. **文件权限问题**
   使用 `dh_install` 或 `debian/install` 正确设置权限
3. **版本号冲突**
   每次构建前更新 `changelog` 中的版本号

通过以上步骤，可以为`Debian/Ubuntu` 系统创建完整的 `.deb` 安装包。实际操作中可能需要根据项目特性进行调整。

# dpkg-buildpackage命令

## 1. 签名相关选项

- **`-uc`**（`--unsigned-changes`）
  不使用 GPG 签名生成的 `.changes` 文件（记录构建过程和包信息的文件）。
- **`-us`**（`--unsigned-source`）
  不使用 GPG 签名源代码包（`.dsc` 文件）。
  **适用场景**：本地开发或测试环境，无需正式发布时。

## 2. 权限模拟选项

- **`-rfakeroot`**
  使用 `fakeroot` 工具模拟 root 权限执行构建过程。
  **作用**：避免真正以 root 身份构建（安全风险），但允许在构建过程中执行需要 root 权限的操作（如创建文件系统层次结构）。
  **依赖**：需预先安装 `fakeroot` 包。

## 3. 架构指定选项

- `-aarmhf`

  指定构建的目标架构为

  ```shell
  armhf #（ARM 32 位硬浮点架构，常见于树莓派等设备）。
  ```

  影响：

  - 编译时使用与 `armhf` 兼容的工具链（如 `arm-linux-gnueabihf-gcc`）；
  - 生成的 `.deb` 包的 `Architecture` 字段标记为 `armhf`，只能安装在对应架构的系统上。

## 4. 编译优化选项

- `-nc`（--no-clean）

  构建前不清理源代码树（即不执行make clean）。

  **适用场景**：

  - 增量构建（节省时间）；
  - 调试构建问题（保留中间文件）。

## 5. 输出控制选项

- **`--changes-option=-quiet`**
  传递 `-quiet` 选项给底层的 `dpkg-genchanges` 工具，减少构建过程中的日志输出（仅显示错误和关键信息）。
  **效果**：使构建日志更简洁，适合自动化环境或批量构建。

## 6.示例

为 ARMhf 架构构建一个 `.deb` 软件包，不进行签名，使用模拟 root 权限，保留现有编译结果，并且输出简洁的构建日志。

```bash
dpkg-buildpackage -uc -us -rfakeroot -aarmhf -nc --changes-option=-quiet
```

