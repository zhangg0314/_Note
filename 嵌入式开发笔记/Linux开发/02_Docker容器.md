

# 1.Docker介绍

## 起源

在传统的软件开发中，开发环境、测试环境和生产环境之间的差异是常见的问题。这种差异可能导致“在我的机器上可以运行，但在别人的机器上不行”的情况。Docker通过容器化技术，将应用程序及其依赖打包到一个独立的环境中，确保在任何支持Docker的系统上都能以一致的方式运行。

- 传统：通过大量的帮助文档，安装程序
- Docker：打包镜像发布测试，一键运行

​	个人理解是：比如在本地的Linux上开发了一个基于ARM的app，而该app用到了各种动态库，而这些动态库显然在编译时需要利用交叉编译，然后再把编译好的动态库，app可执行文件等各种配置依赖再放到运行着Linux的开发板上去。也就是当有很多开发板需要运行这个app时，每次都需要为这个开发板移植相同的配置环境，这很麻烦。而通过Docker技术，把app和依赖环境一起打包成一个容器镜像，再放到各个开发板上去基于该进行创建容器并运行就可以直接运行了，就是所谓的“一次配置，处处运行”。![image-20250312164209682](..\figure\image-20250312164209682.png)

## Docker与虚拟机技术

​	虚拟机就是在window中装一个Vmware，通过这个软件我们可以虚拟出来一台或者多台电脑！（很笨重）虚拟机属于虚拟化技术，Docker容器技术，也是一种虚拟化技术。

- 传统的虚拟机，可以虚拟出一条硬件，运行一个完整的操作系统，在这个操作系统上安装和运行所需的软件
- 容器内的应用可以直接运行在宿主 主机的内核中，容器没有自己的内核，也不用虚拟硬件 （轻便）
- 每个容器是相互隔离的，每个容器内都有属于自己的文件系统，之间互不影响。
- Docker是内核级别的虚拟化，可以在一个物理机上运行很多的容器，让服务器的性能可以压榨到极致。

# 2.基本组成

​	Docker 使用客户端-服务器 (C/S) 架构模式。Docker 客户端会与 Docker 守护进程进行通信。Docker 守护进程会处理复杂繁重的任务，例如建立、运行、发布 Docker 容器。Docker 客户端和守护进程可以运行在同一个系统上，当然你也可以使用 Docker 客户端去连接一个远程的 Docker 守护进程。Docker 客户端和守护进程之间通过 socket 或者 RESTful API 进行通信。

##  守护进程

​	Docker 守护进程运行在一台主机上。用户并不直接和守护进程进行交互，而是通过 Docker 客户端间接和其通信。

## 客户端

​	Docker 客户端，实际上是 docker 的二进制程序，是主要的用户与 Docker 交互方式。它接收用户指令并且与背后的 Docker 守护进程通信，如此来回往复。

## 内部组成

- **镜像（Image）**——静态

  docker镜像就好比一个模板，我们可以通过这个模板来创建容器服务，通过这个镜像可以创建多个容器（最终服务运行或者项目运行就是在容器中的）。每一个镜像由一系列的层 (layers) 组成。Docker 使用 UnionFS 来将这些层联合到单独的镜像中。当你改变了一个 Docker 镜像，比如升级到某个程序到新的版本，一个新的层会被创建。因此，不用替换整个原先的镜像或者重新建立(在使用虚拟机的时候你可能会这么做)，只是一个新的层被添加或升级了。现在你不用重新发布整个镜像，只需要升级，层使得分发 Docker 镜像变得简单和快速。

  **大多数Docker镜像基于Linux，因此拉取下来后都快通过创建容器并利用bash进行交互，像Python、Mysql等其文件系统就是精简版的linux文件系统。因此可以运用一些基本的Linux命令**。

  说白了就是Python镜像就是搭建了运行Python的Linux，Mysql镜像就是搭建了运行Mysql环境的Linux。

- **容器（container）**——动态

  docker利用容器技术，独立运行一个或者一组应用通过镜像来创建启动，停止，删除，基本命令。Docker 容器是 Docker 的运行部分。

- **仓库（repository）**

  仓库就是存放镜像（image）的地方，可以理解为代码控制中的代码仓库。仓库又可以分为公有仓库和私有仓库。

# 3.Docker安装

## 卸载旧版本

在安装 Docker Engine 之前，需要卸载任何冲突的软件包。正在在使用的Linux 发行版可能提供非官方的 Docker 软件包，当使用 Docker 提供的官方软件包时这可能会发生冲突。因此必须在安装 Docker Engine 正式版之前卸载这些软件包 。

要卸载的非官方软件包是：

- `docker.io`
- `docker-compose`
- `docker-compose-v2`
- `docker-doc`
- `podman-docker`

此外，Docker Engine 是由自己和它所依赖的依赖项捆绑成的一个捆绑包：如果之前已安装,那么卸载它们以避免 与 Docker Engine 捆绑的版本冲突。
`containerd``runc``containerd.io``containerd``runc`

执行以下命令卸载所有冲突的软件包。

```bash
for pkg in docker.io docker-doc docker-compose docker-compose-v2 podman-docker containerd runc; 
    do 
    	sudo apt-get remove $pkg; #apt-get可能会报告没有安装这些软件包。
    done										
```

存储在其中的映像、容器、卷和网络不会在卸载 Docker 时自动删除。如果想安装全新的版本，并希望清理任何现有数据，删除`/var/lib/docker/`

## Docer的安装

[Docker CE 镜像源站-阿里云开发者社区 (aliyun.com)](https://developer.aliyun.com/article/110806)

1. 卸载旧版本（如果存在）

   ```bash
   sudo apt-get remove docker docker-engine docker.io containerd runc
   ```

2. 更新apt包索引

   ```Bash
   sudo apt-get update
   ```

3. 安装必要的依赖包，由于`apt` 源使用 `HTTPS `以确保软件下载过程中不被篡改。因此，我们首先需要添加使用 HTTPS 传输的软件包以及 CA 证书。

   ```Bash
   sudo apt-get -y install apt-transport-https ca-certificates curl software-properties-common
   ```

4. 鉴于国内网络问题，强烈建议使用国内源，官方源请在注释中查看。

   为了确认所下载软件包的合法性，需要添加软件源的 `GPG` 密钥

   ```bash
   #安装GPG证书
   curl -fsSL http://mirrors.aliyun.com/docker-ce/linux/ubuntu/gpg | sudo apt-key add -
   ```

5. 添加 Docker 软件源，添加稳定版本的 Docker APT 镜像源

   ```bash
   sudo add-apt-repository "deb [arch=amd64] https://mirrors.aliyun.com/docker-ce/linux/ubuntu $(lsb_release -cs) stable"#推荐使用国内镜像源，不然很有可能会连接超时
   ```

6. 再次更新apt包索引(因为前面增加了新连接，需要更新)

   ```bash
   sudo apt-get update
   ```

7. 安装最新版本的Docker Engine-Community和containerd

   ```bash
   sudo apt-get install docker-ce docker-ce-cli containerd.io
   #ce 表示社区版引擎
   ```

8. 验证Docker是否安装成功

   ```bash
   sudo docker run hello-world
   #这里hello-world是image的名字，执行此指令时，docker首先会在本地找是否有hello-world的镜像，如果有，则直接利用其创建容器，如果没有则去Docker Hub上找，找到下载到本地并创建容器，没找到则报错。
   ```

## 安装问题

docker: Error response from daemon: Get "https://registry-1.docker.io/v2/": net/http: request canceled while waiting for connection (Client.Timeout exceeded while awaiting headers)。也就是连接超时，直接多加几个镜像网站，提高有用命中率。

尝试修改一下docker配置文件：/etc/docker/daemon.json (以Ubuntu Server为例），改成或增加下面的内容：

```vim
{
  "registry-mirrors": ["https://docker-cf.registry.cyou",
"https://docker.jsdelivr.fyi",
"https://docker.rainbond.cc",
"https://docker.registry.cyou",
"https://dockertest.jsdelivr.fyi",
"https://mirror.aliyuncs.com",
"https://dockercf.jsdelivr.fyi",
"https://dockerproxy.com",
"https://docker.mirrors.ustc.edu.cn",
"https://mirror.baidubce.com",
"https://docker.m.daocloud.io",
"https://docker.nju.edu.cn",
"https://docker.mirrors.sjtug.sjtu.edu.cn",
"https://mirror.iscas.ac.cn"]
}
```

然后，执行以下命令。重启docker服务：

```bash
sudo systemctl daemon-reload && sudo systemctl restart docker
```

进行以上操作后，再重新拉取镜像试试。

# 4.Docker常用命令

## 帮助命令

```bash
sudo docker version           # 显示docker的版本信息
sudo docker info              # 显示docker的系统信息，包括镜像和容器的数量
sudo docker 命令 --help        # 帮助命令
```

## 镜像命令

### 查看镜像

```bash
docker images
	-a, --all # 显示所有镜像 (docker images -a)
	-q, --quiet # 仅显示镜像id (docker images -q)
	REPOSITORY            TAG               IMAGE ID            CREATED             SIZE
#   镜像的仓库源   镜像的标签(版本号或latest)   镜像的id           镜像的创建时间       镜像的大小
```

### 搜索镜像

```bash
docker search 镜像名
```

### 打标签

```bash
 docker tag SOURCE_IMAGE[:TAG] TARGET_IMAGE[:TAG]
#Create a tag TARGET_IMAGE that refers to SOURCE_IMAGE，是又创建了一个镜像，不是替换原来的镜像

```

### 拉取镜像

```bash
docker pull 镜像名[:tag]
```

### 删除镜像

```bash
docker rmi -f 镜像名 #强制删除
```

## 容器命令

### 新建容器并启动

```bash
docker run [OPTIONS] IMAGE [COMMAND] [ARG...]
docker run [option] 镜像名

#option
  	-e, --env list  #Set environment variables
	--name = "NAME"  #指定容器名字
	-d			    #后台方式运行
	-it				#使用交互式命令进入容器，要使用bash终端 /bin/bash
	-p				#指定容器端口
		-p 主机端口:容器端口 #映射主机端口到容器端口，端口暴露，外部可以通过访问指定的主机的ip和端口来间接访问该主机的该容器上的指定端口。这样外部访问容器服务就好像直接访问这条机器一样
		-p 容器端口
	-P 				#随机指定端口
```

### 启动和停止容器

```bash
docker start 容器id
docker restart 容器id
docker stop 容器id
docker kill 容器id
```

### 列出所有正在运行的容器

```bash
docker ps 
 	 #默认列出正在运行的容器
 -a  #列出正在运行的容器 + 历史运行过的容器
 -n = ？ #显示最近创建的？个容器
 -q      #只显示容器编号
```

### 进入当前正在运行的容器

```bash
#方式1：进入容器并执行bash命令-----开启一个新的终端
docker exec [OPTIONS] CONTAINER COMMAND [ARG...]
docker exec -it 容器id /bin/bash

#方式2：-----进入一个容器当前正在运行的终端
docker attach [OPTIONS] CONTAINER
docker attach 容器id
```

### 退出容器

```bash
exit #退出并停止容器
Ctrl + P + Q #容器不停止地退出
```

### 删除容器

```bash
docker rm  容器id 	#删除指定id的容器，不能删除正在运行的容器
docker rm -f 容器id  #强制删除容器，包括正在运行的
```

### 全部删除（骚操作）

```bash
 docker rm $(docker ps -aq)
```

## 常用其他命令

### 查看容器运行CPU状态

```bash
docker stats
```

### 后台启动容器

```bash
docker run -d 镜像名

#问题:docker ps后发现镜像直接停止了
#常见的坑:容器使用后台运行，就必须要有一个前台进行来为其提高服务，不然docker发现没有服务要提供，就会停止后台容器。
```

### 显示日志命令

```bash
docker logs [option] 容器名

#option
    -t, --timestamps     Show timestamps
	-f --follow #实时动态追踪日志的输出
	-n, --tail string    #Number of lines to show from the end of the logs (default "all")
```

### 查看容器中进程信息

```bash
docker top 容器id
UID   PID    PPID   C   STIME     TTY        TIME                CMD
root  50934  50911  0   16:00     pts/0      00:00:00            bash
```

### 查看镜像的元数据

```bash
docker inspect 容器/镜像id


[
    {
        "Id": "sha256:94124e1b0412d8ea681c370b2724986703ec380e867240c13247050b3ea39ca2",
        "RepoTags": [
            "python:slim"
        ],
        "RepoDigests": [
            "python@sha256:21e39cf1815802d4c6f89a0d3a166cc67ce58f95b6d1639e68a394c99310d2e5"
        ],
        "Parent": "",
        "Comment": "buildkit.dockerfile.v0",
        "Created": "2025-04-08T19:02:43Z",
        "DockerVersion": "",
        "Author": "",
        "Config": {
            "Hostname": "",
            "Domainname": "",
            "User": "",
            "AttachStdin": false,
            "AttachStdout": false,
            "AttachStderr": false,
            "Tty": false,
            "OpenStdin": false,
            "StdinOnce": false,
            "Env": [
                "PATH=/usr/local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin",
                "GPG_KEY=7169605F62C751356D054A26A821E680E5FA6305",
                "PYTHON_VERSION=3.13.3",
                "PYTHON_SHA256=40f868bcbdeb8149a3149580bb9bfd407b3321cd48f0be631af955ac92c0e041"
            ],
            "Cmd": [
                "python3"
            ],
            "Image": "",
            "Volumes": null,
            "WorkingDir": "",
            "Entrypoint": null,
            "OnBuild": null,
            "Labels": null
        },
        "Architecture": "amd64",
        "Os": "linux",
        "Size": 120751291,
        "GraphDriver": {
            "Data": {
                "LowerDir": "/var/lib/docker/overlay2/882bd5fb9a94025ace766ffedcde818fd7b700f73f69255e64e12d8572932c11/diff:/var/lib/docker/overlay2/5098dcd12172e5d0936cf8dca39227a3a815018ace0ab54385fb8e07ad937f70/diff:/var/lib/docker/overlay2/a6e5864ba1bdc961afa9fe2bf824ede5450d03e6aa0889335c22864f5858495b/diff",
                "MergedDir": "/var/lib/docker/overlay2/38e5fa950c1ef37abd48e80335d2fb456e10328dfd332fb5c4e9bf5b8f5d7e2d/merged",
                "UpperDir": "/var/lib/docker/overlay2/38e5fa950c1ef37abd48e80335d2fb456e10328dfd332fb5c4e9bf5b8f5d7e2d/diff",
                "WorkDir": "/var/lib/docker/overlay2/38e5fa950c1ef37abd48e80335d2fb456e10328dfd332fb5c4e9bf5b8f5d7e2d/work"
            },
            "Name": "overlay2"
        },
        "RootFS": {
            "Type": "layers",
            "Layers": [
                "sha256:ea680fbff095473bb8a6c867938d6d851e11ef0c177fce983ccc83440172bd72",
                "sha256:0dbd92ca4322ca1d1a8db29aae3995c24b1a5416e807dfd6d03eafbf107ba9be",
                "sha256:20762b83e049e67c36eb5bf05dd09021e071c89f5fd237c3a03370b208e903b0",
                "sha256:e8e6022208c3184852d232636bd5850ad0924428696620758b5d585f1eb981a1"
            ]
        },
        "Metadata": {
            "LastTagTime": "0001-01-01T00:00:00Z"
        }
    }
]
```

### 容器内文件拷贝到主机上

```bash
docker cp 容器id:/path/to/file /localhost/path/
```

## 总结框图

![image-20250412163420441](..\figure\image-20250412163420441.png)

# 5.Docker镜像

## 镜像是什么

镜像是一种轻量级、可执行的独立软件包，用来打包运行环境和基于运行环境开发的软件，它包含运行某个软件所需的所有内容，包括代码、运行时的库、环境变量和配置文件等。

所有的应用，直接打包成docker镜像，就可以直接在任何地方部署运行！

## 联合文件系统

​    Union文件系统（UnionFS）是一种分层、轻量级并且高性能的文件系统，它支持对文件系统的修改作为一次提交来一层层的叠加，同时可以将不同目录挂载到同一个虚拟文件系统下（unite several directories into a single virtual filesystem）。Union 文件系统是Docker 镜像的基础。镜像可以通过分层来进行继承，基于基础镜像（没有父镜像），可以制作各种具体的应用镜像。

​    **特性：**一次同时加载多个文件系统，但从外面看起来，只能看到一个文件系统，联合加载会把各层文件系统叠加起来，这样最终的文件系统会包含所有底层的文件和目录。

## 镜像加载原理

**bootfs（boot file system）**主要包含bootloader和kernel，bootloader主要是引导加载kernel，Linux刚启动时会加载bootfs文件系统，在Docker镜像的最底层是bootfs。这一层与我们典型的Linux/Unix系统是一样的，包含boot加载器和内核。当boot加载完成之后整个内核就都在内存中了，此时内存的使用权已由bootfs转交给内核，此时系统也会卸载bootfs。

**rootfs（root file system）**，在bootfs之上。包含的就是典型Linux系统中的/dev，/proc，/bin，/etc等标准目录和文件。
rootfs就是各种不同的操作系统发行版，比如Ubuntu，Centos等等。

对于一个精简的OS，rootfs可以很小，只需要包含最基本的命令、工具和程序库就可以了。因为底层直接用Host的kernel，自己只需要提供rootfs就可以了。由此可见对于不同的linux发行版，bootfs基本是一致的，rootfs会有差别，因此不同的发行版可以公用bootfs。  ![img](..\figure\0589875ffdf48da8f60e61cea09bd79f.png)

# 6.数据卷（有-v 主机-容器）

Docker 中的数据卷（**Volume**）是一种用于**持久化存储容器数据**的机制。它的核心目的是解决容器内数据易失性的问题（容器停止或删除后，其内部数据会丢失），同时支持容器与宿主机、容器与容器之间的数据共享。

数据卷也就是目录挂载+持久化存储，即共享目录+拷贝一份到本地，操作被挂载的主机目录就是在操作容器目录，可以这个挂载理解为双向绑定，把本地主机目录的文件同步到容器中的同时也把容器中的文件同步过来。

---

## 核心概念
1. **持久化存储** 
   数据卷独立于容器的生命周期存在，即使容器被删除，数据卷中的数据依然保留。这使得数据可以长期保存、重复使用。
2. **解耦数据与容器** 
   将数据存储在容器外部（如宿主机的特定目录或云存储），避免容器镜像变得臃肿，同时提升安全性（数据与容器分离）。
3. **高效数据共享** 
   多个容器可以挂载同一个数据卷，实现数据共享（例如：Web 服务器和日志处理容器共享日志目录）。

---

## 常见用途
- **数据库存储**：如 MySQL 的数据库文件（避免容器重启后数据丢失）。
- **配置文件**：将应用的配置文件挂载到容器中，方便修改。
- **日志文件**：集中存储容器产生的日志。
- **静态资源**：Web 应用的静态文件（如 HTML、CSS、JS）。

---

## 两种类型
### 命名卷（Named Volumes）

- 由 Docker 管理的存储位置（通常位于宿主机的 `/var/lib/docker/volumes/_data`）。
- **优势**：易用性强，适合不关心物理路径的场景。
- **示例**：
  
  ```bash
  # 创建命名卷
  docker volume create my_volume
  
  # 挂载到容器
  docker run -d -v my_volume:/path/in/container nginx
  ```

### 绑定挂载（Bind Mounts）

- 直接将宿主机的目录或文件挂载到容器中。

- **优势**：直接控制宿主机路径，适合开发调试（如挂载代码目录）。

- **示例**：
  ```bash
  # 将宿主机的 /host/path 挂载到容器的 /container/path
  docker run -d -v /host/path:/container/path nginx
  ```

---

## 卷相关操作命令
### 创建数据卷

```bash
docker volume create my_volume
```

### 查看所有数据卷

```bash
docker volume ls

[linux@linux-vir--mac]:~$ docker volume ls
DRIVER    VOLUME NAME
local     0ef6bba1600b27b5e9e47e19c4373416923e3678dfb700f92b3f9d2d23cff97b
local     ab25279fa22024d1c8e499c38c8dcf1a6a676372a2021cf5921a73a02e090a19
```

### 删除数据卷

```bash
docker volume rm my_volume
```

### 启动容器并挂载数据卷

```bash
docker run -d --name my_container -v my_volume:/app/data nginx

docker run -v 	
	-v 主机目录:容器目录 
#example
#Linux服务器中
[linux@linux-vir--mac]:~$ docker run -it --rm tomcat:9.0 #用完即删，即退出容器后docker ps看不到该容器了 
[linux@linux-vir--mac]:~$ docker run -itd --name zgdev -v /home/zhangg/driver:/home/ubuntu -v /opt/ext-toolchain:/ext-toolchain smartchip-sdk-build:v0.9 /bin/bash
[linux@linux-vir--mac]:~$ docker exec -it zgdev bash

#容器中
[root@0e2d614a9844]:~$ cd lichee-smios-sdk
[root@0e2d614a9844]:~$ export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/ext-toolchain/bin
[root@0e2d614a9844]:~$./set_board.sh jzq

[root@0e2d614a9844]:~$./build.sh

[root@0e2d614a9844]:~$./build.sh pack
```

---

## 数据卷 vs 绑定挂载
| **特性** | **数据卷（Volume）**      | **绑定挂载（Bind Mount）**   |
| -------- | ------------------------- | ---------------------------- |
| 存储位置 | Docker 管理的目录（隐藏） | 用户指定的宿主机目录（显式） |
| 可移植性 | 高（适合生产环境）        | 低（依赖宿主机目录结构）     |
| 权限控制 | 自动处理文件权限          | 需手动处理权限问题           |
| 适用场景 | 持久化数据、数据库存储    | 开发调试、挂载配置文件       |

---

## 实际应用场景示例
### 持久化数据库

```bash
docker run -d --name mysql_db -v mysql_data:/var/lib/mysql -e MYSQL_ROOT_PASSWORD=123 mysql
```
即使容器被删除，数据仍保留在 `mysql_data` 卷中。

### 开发环境代码热更新

```bash
docker run -d --name dev_app -v $(pwd)/code:/app python:3.9
```
修改宿主机 `code` 目录中的代码，容器内 `/app` 会实时同步。

## 具名和匿名挂载

### 匿名挂载

```bash
-v #只写容器内路径，省略主机路径
docker run -d -P --name nginx01 -v /etc/nginx nginx
```

### 具名挂载

```bash
-v #只写容器内路径，省略主机路径,但给挂载卷取个名字，不能加/，不然被翻译为了主机路径了
docker run -d -P --name nginx01 -v juming:/etc/nginx nginx

DRIVER    VOLUME NAME
local     0ef6bba1600b27b5e9e47e19c4373416923e3678dfb700f92b3f9d2d23cff97b
local     ab25279fa22024d1c8e499c38c8dcf1a6a676372a2021cf5921a73a02e090a19
local     juming
```

## 数据卷读写权限

```bash
# 通过 -v 容器内的路径:ro    rw    改变读写权限
ro    read only    # 只读
rw    read write    # 可读可写
# 一旦设置了容器权限，容器对我们挂载出来的内容就有了限定。
docker run -d -p 3344:80 --name nginx02 -v juming-nginx:/etc/nginx:ro nginx
docker run -d -p 3344:80 --name nginx02 -v juming-nginx:/etc/nginx:rw nginx
# 只要看到ro就说明这个路径只能通过宿主机来操作，容器内部是无法操作！
```

## 总结

- 数据卷是 Docker 中管理持久化数据的核心工具，它通过分离数据与容器生命周期，解决了容器内数据易失性的问题。无论是生产环境的数据持久化，还是开发环境的快速调试，数据卷都能提供灵活可靠的存储方案。

- 所有的docker容器内的卷，没有指定目录的情况下都是在`/var/lib/docker/volumes/xxxx/_data"`（xxxx是卷名）。我们通过具名挂载可以方便的找到我们的一个卷，`大多数情况在使用的具名挂载`

- 如何确定是具名挂载，还是匿名挂载，还是指定路径挂载

  ```bash
  -v 容器内的路径             # 匿名挂载
  -v 卷名:容器内的路径        # 具名挂载
  -v /宿主机路径:容器内路径    # 指定路径挂载
  ```

# 7.数据卷容器（无-v 容器-容器）

## 问题引出

两个Mysql容器共享同一份数据或者多个服务器容器共享同一个日志目录？如何进行容器到容器间的挂载？

## 多个容器同步数据（临时认父）

![img](..\figure\65e153888926d92ed1162393ca2c13a2.png)



## 实践步骤

1. 先通过Dockerfile的`volume`添加卷，并以此构建镜像

   ```vim
   FROM python:slim
   WORKDIR	/
   VOLUME /home/test
   RUN /bin/bash
   ```
   
1. 创建父容器

   ```bash
   #父容器，直接基于镜像创建
   docker run -it --name=continer1 myimage /bin/bash
   [root@6a0d28483ee0]:/$ ls
   bin  boot  dev	etc  home  lib	lib64  media  mnt  opt	proc  root  run  sbin  srv  sys  tmp  usr  var
   [root@6a0d28483ee0]:/$ cd home/
   [root@6a0d28483ee0]:/home$ ls
   test
   
   #
           "Mounts": [
               {
                   "Type": "volume",
                   "Name": "de1f346a5142a3309e629f44ab6ce44a84b529f0ba8cb07af12fe6a08aaa9275",
                   "Source": "/var/lib/docker/volumes/de1f346a5142a3309e629f44ab6ce44a84b529f0ba8cb07af12fe6a08aaa9275/_data",
      #匿名挂载
                   "Destination": "/home/test",
                   "Driver": "local",
                   "Mode": "",
                   "RW": true,
                   "Propagation": ""
               }
   
   ```
   
3. 创建子容器

   ```bash
   docker run -it --name continer2 --volumes-from continer1 myimage /bin/bash
   [root@38367a7e1d5b]:/# ls
   bin  boot  dev	etc  home  lib	lib64  media  mnt  opt	proc  root  run  sbin  srv  sys  tmp  usr  var
   [root@38367a7e1d5b]:/# cd home/
   [root@38367a7e1d5b]:/home# ls
   test
   [root@38367a7e1d5b]:/home# cd test
   [root@38367a7e1d5b]:/home/test# ls
   [root@38367a7e1d5b]:/home/test# touch 1.txt
   [root@38367a7e1d5b]:/home/test# ls
   1.txt
   [root@38367a7e1d5b]:/home/test# 
   
   #查看父容器
   [linux@linux-vir--mac]:~/Docker$ docker attach continer1
   [root@6a0d28483ee0]:/# cd home/test/
   [root@6a0d28483ee0]:/home/test# ls
   1.txt
   ```

## 总结

1. 容器之间挂载用 `--volume from`，容器与主机之间的挂载用`docker run -v`。
2. 容器之间配置信息的传递，数据卷容器的生命同期一直持续到没有容器使用为止。但是一旦持久化到了本地，这个时候，本地的数据是不会删除的！
3. 无论创建几个容器，各个容器的数据是互相拷贝的删除任意一个，其余的容器都可以看到那些数据。当然所有的容器都删了也就会删除了，除非做了持久化映射 ![image-20250413162923121](..\figure\image-20250413162923121.png)

# 8.Dockerfile

​    Docker Hub中99%镜像都是从这个基础镜像过来的FROM scratch，然后配置需要的软件和配置来进行的构建。

## 初识dockerfile

Dockerfile：构建文件，定义了构建镜像的一切步骤，源代码

DockerImages：通过DockerFile构建生成的镜像，最终发布和运行的产品

Docker容器：容器就是基于镜像运行起来提供服务的。

`dockerfile `就是用来构建 docker 镜像的构建文件，是一个命令脚本，通过这个脚本可以生成镜像，镜像是一层一层的，脚本一个一个的命令构成一层层镜像，每个命令都是一层！

## Dockerile语法

### 基础知识

1. 每个保留关键字（指令）都必须是大写字母

2. Dockerfile语句执行从上到下顺序执行

3. #表示注释

4. 每个指令都会创建提交一个新的镜像层，并提交！

   ![image-20250413132433613](..\figure\image-20250413132433613.png)

### DockerFile指令

```dockerfile
FROM 				#基础镜像，一切从这里开始构建
MAINTAINER			#维护者信息，镜像是谁写的，姓名+邮箱
RUN					#镜像构建的时候需要运行的命令
ADD					#添加服务如tomcat，mysql等的压缩包（会自动解压）
WORKDIR				#镜像的工作目录
VOLUME				#设置卷，挂载目录
EXPOSE				#暴露容器自身的端口
CMD 				#指定容器启动时要运行的命令，如/bin/bash，命令会被外部命令替换
ENTRYPOINT			#指定容器启动时要运行的命令，命令会被外部命令追加
ONBUILD				#当构建一个被继承DockerFile这个时候就会运行ONBUILD的指令，触发命令
COPY				#类似ADD,将文件拷贝到镜像
ENV					#构建的时候设置环境变量
```

## 构建镜像步骤

1. 创建`requirement.tx`**指定版本**
   ```vim
   
   ```

2. 编写DockerFile文件,官方默认文件名`Dockerfile`(无需指定文件)

   ```dockerfile
   FROM python:slim 
   
   MAINTAINER	zhangg<z3254406361@163.com>
   
   WORKDIR /
   
   COPY README.md /app/dlms_test
   ADD dlms_pyhon_test.tar.gz /app/dlms_test #会自动解压
   
   RUN apt-get install
   
   ENV PATH=XXX:$PATH
   
   CMD /bin/bash && cd /app
   ```

3. **执行构建命令**

   ```bash
   docker build [OPTIONS]  PATH | URL | -
   #PATH .
   # options
   	-f,--file string #Name of the Dockerfile (default: "PATH/Dockerfile")
   	-t, --tag stringArray               Name and optionally a tag (format: "name:tag")
   ```

4. **显示镜像历史构建过程**

   ```bash
   docker history [OPTIONS] IMAGE
   #可以研究某个镜像是怎么做的
   ```

## CMD和ENTRYPOINT区别

```bash
CMD            # 指定这个容器启动的时候要运行的命令，只有最后一个会生效，可被替代
ENTRYPOINT     # 指定这个容器启动的时候要运行的命令，可以追加命令

docker run 镜像 -l#运行容器时带的参数
```

```bash
# 原因：ENTRYPOINT命令的情况下，"-l"追加在ENTRYPOINT ["1s"，"-a"]命令后面，得到"ls -al"的命令，所以命令正常执行！
# （我们的追加命令，是直接拼接在我们的ENTRYPOINT命令的后面）
#而CMD会之间把ls -a替换成-l，而-l命令Linux没有会报错
```

# 9.发布镜像到服务器

1. 在服务器注册账号并登录

   ```bash
   docker login [option] [SERVER]
   #options
   	-p，--password
   	-u,--username
   ```

2. 在服务器上提交自己的镜像

   ```bash
   docker push [OPTIONS] Author/NAME[:TAG]
   
   #尽量带上作者信息和镜像版本号
   ```

3. 然后其他人通过网站访问服务器

4. 拉取服务器镜像，创建容器

5. 这也就实现了网站公司发布服务，客户直接下载运行服务的效果了。

# 10.Docker网络

## 理解docker0

## --link（单向的）

通过服务名来ping，比如容器A --link 容器B的本质就是在A的`/etc/host	`配置中增加了一个容器B的ip 和容器B的名字，因此可以直接通过服务名来ping。

但由于官方的docker0网卡有许多问题比如现在这个不能ping服务名。因此现在使用的最多的是自定义的网卡而不用dokcer0

### 自定义网络

#### 查看所有的docker网络

```bash
docker network ls
```

#### 网络模式

- bridge:桥接docker（默认，自定义网络也使用此模式）
- NAT
- host
- container （容器特有的）

#### 创建网络

``` bash
docker network create [option] 网络名
#option
 	--driver bridge #桥接模式
 	--subnet 192.167.65.0/16#子网
 	--gateway 192.167.65.1/16#网关
 
```



# 11.可视化

## 1.Portainer

Portainer是Docker的图形化界面管理工具！提供一个后台面板给我们操作！

## 2.Rancher（CI/CD）

# 12.参考链接

[1].https://yeasy.gitbook.io/docker_practice/os/busybox

[2].[Docker入门教程（非常详细）从零基础入门到精通，看完这一篇就够了_docker教程-CSDN博客](https://blog.csdn.net/leah126/article/details/131871717?ops_request_misc=%7B%22request%5Fid%22%3A%229c0fd340e72d327f2ab6683174acf0b3%22%2C%22scm%22%3A%2220140713.130102334..%22%7D&request_id=9c0fd340e72d327f2ab6683174acf0b3&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~sobaiduend~default-7-131871717-null-null.nonecase&utm_term=docker&spm=1018.2226.3001.4450)