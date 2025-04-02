

# 1.Docker介绍

## 1.起源

​	在传统的软件开发中，开发环境、测试环境和生产环境之间的差异是常见的问题。这种差异可能导致“在我的机器上可以运行，但在别人的机器上不行”的情况。Docker通过容器化技术，将应用程序及其依赖打包到一个独立的环境中，确保在任何支持Docker的系统上都能以一致的方式运行。

- 传统：通过大量的帮助文档，安装程序
- Docker：打包镜像发布测试，一键运行

​	个人理解是：比如在本地的Linux上开发了一个基于ARM的app，而该app用到了各种动态库，而这些动态库显然在编译时需要利用交叉编译，然后再把编译好的动态库，app可执行文件等各种配置依赖再放到运行着Linux的开发板上去。也就是当有很多开发板需要运行这个app时，每次都需要为这个开发板移植相同的配置环境，这很麻烦。而通过Docker技术，把app和依赖环境一起打包成一个容器镜像，再放到各个开发板上去基于该进行创建容器并运行就可以直接运行了，就是所谓的“一次配置，处处运行”。![image-20250312164209682](D:\MyNote\-\嵌入式开发笔记\figure\image-20250312164209682.png)

## 2.Docker与虚拟机技术的区别联系

​	虚拟机就是在window中装一个Vmware，通过这个软件我们可以虚拟出来一台或者多台电脑！（很笨重）虚拟机属于虚拟化技术，Docker容器技术，也是一种虚拟化技术。

- 传统的虚拟机，可以虚拟出一条硬件，运行一个完整的操作系统，在这个操作系统上安装和运行所需的软件
- 容器内的应用可以直接运行在宿主 主机的内核中，容器没有自己的内核，也不用虚拟硬件 （轻便）
- 每个容器是相互隔离的，每个容器内都有属于自己的文件系统，之间互不影响。
- Docker是内核级别的虚拟化，可以在一个物理机上运行很多的容器，让服务器的性能可以压榨到极致。

# 2.基本组成

​	Docker 使用客户端-服务器 (C/S) 架构模式。Docker 客户端会与 Docker 守护进程进行通信。Docker 守护进程会处理复杂繁重的任务，例如建立、运行、发布 Docker 容器。Docker 客户端和守护进程可以运行在同一个系统上，当然你也可以使用 Docker 客户端去连接一个远程的 Docker 守护进程。Docker 客户端和守护进程之间通过 socket 或者 RESTful API 进行通信。

## 1.Docker 守护进程

​	Docker 守护进程运行在一台主机上。用户并不直接和守护进程进行交互，而是通过 Docker 客户端间接和其通信。

## 2.Docker 客户端

​	Docker 客户端，实际上是 docker 的二进制程序，是主要的用户与 Docker 交互方式。它接收用户指令并且与背后的 Docker 守护进程通信，如此来回往复。

## 3.Docker 内部

- **镜像（Image）**——静态

  docker镜像就好比一个模板，我们可以通过这个模板来创建容器服务，通过这个镜像可以创建多个容器（最终服务运行或者项目运行就是在容器中的）。每一个镜像由一系列的层 (layers) 组成。Docker 使用 UnionFS 来将这些层联合到单独的镜像中。当你改变了一个 Docker 镜像，比如升级到某个程序到新的版本，一个新的层会被创建。因此，不用替换整个原先的镜像或者重新建立(在使用虚拟机的时候你可能会这么做)，只是一个新的层被添加或升级了。现在你不用重新发布整个镜像，只需要升级，层使得分发 Docker 镜像变得简单和快速。

- **容器（container）**——动态

  docker利用容器技术，独立运行一个或者一组应用通过镜像来创建启动，停止，删除，基本命令。Docker 容器是 Docker 的运行部分。

- **仓库（repository）**

  仓库就是存放镜像（image）的地方，可以理解为代码控制中的代码仓库。仓库又可以分为公有仓库和私有仓库。

# 3.Docker安装

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
   sudo apt-get install apt-transport-https ca-certificates curl gnupg-agent software-properties-common
   ```

4. 鉴于国内网络问题，强烈建议使用国内源，官方源请在注释中查看。

   为了确认所下载软件包的合法性，需要添加软件源的 `GPG` 密钥

   ```bash
   curl -fsSL https://mirrors.aliyun.com/docker-ce/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg
   #使用国内阿里云镜像源（国内用户推荐）
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
   ```

8. 验证Docker是否安装成功

   ```bash
   sudo docker run hello-world
   #这里hello-world是image的名字，执行此指令时，docker首先会在本地找是否有hello-world的镜像，如果有，则直接利用其创建容器，如果没有则去Docker Hub上找，找到下载到本地并创建容器，没找到则报错。
   ```

# 4.Docker常用命令

## 1.帮助命令

```bash
docker version           # 显示docker的版本信息
docker info              # 显示docker的系统信息，包括镜像和容器的数量
docker 命令 --help        # 帮助命令
```

## 2.镜像命令

```bash
docker images
	-a, --all # 显示所有镜像 (docker images -a)
	-q, --quiet # 仅显示镜像id (docker images -q)
	REPOSITORY            TAG               IMAGE ID            CREATED             SIZE
#   镜像的仓库源   镜像的标签(版本号或latest)   镜像的id           镜像的创建时间       镜像的大小
```

docker run -itd --name zgdev -v /home/zhangg/driver:/home/ubuntu -v /opt/ext-toolchain:/ext-toolchain smartchip-sdk-build:v0.9 bash
 2002  docker exec -it zgdev bash



cd lichee-smios-sdk/
    4  export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/ext-toolchain/bin
    5  ./set_board.sh jzq
    6  ./build.sh 
    7  ./build.sh pack

# 参考链接

[1].https://yeasy.gitbook.io/docker_practice/os/busybox