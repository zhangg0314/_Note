FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# 换阿里源 + 基础依赖
RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential zip unzip wget curl \
        dpkg-dev debhelper autotools-dev fakeroot \
        && rm -rf /var/lib/apt/lists/*

# ARM 交叉工具链 (Linaro GCC 7.4.1)
COPY gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf \
     /opt/gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf

# 源码编译的指定版本 (autoconf 2.69 / automake 1.14.1 / libtool 2.4.2)
COPY usr-local /usr/local

ENV PATH="/opt/gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf/bin:/usr/local/bin:${PATH}"
ENV CROSS_COMPILE=arm-linux-gnueabihf-
ENV ARCH=arm

WORKDIR /workspace
