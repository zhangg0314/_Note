# MQTT协议

## 1.MQTT简介

什么是MQTT，他的英文全称Message Queuing Telemetry Transport，翻译过来就是“消息队列遥测传输”，是ISO 标准(ISO/IEC PRF 20922)下基于发布/订阅范式的消息协议。它工作在 TCP/IP协议族上，是为硬件性能低下的远程设备以及网络状况糟糕的情况下而设计的发布/订阅型消息协议。
目前支持MQTT的服务器中间件有[EMQTT](https://www.cnblogs.com/zhumengke/articles/11226743.html)、Mosquitto、Apollo。

## 2.MQTT特点

MQTT协议是为大量计算能力有限，且工作在低带宽、不可靠的网络的远程传感器和控制设备通讯而设计的协议，它具有以下主要的几项特性：

- 使用发布/订阅消息模式，提供一对多的消息发布，解除应用程序耦合；

- 对负载内容屏蔽的消息传输；

- 使用 TCP/IP 提供网络连接；

- 有三种消息发布服务质量，QOS消息服务质量默认是0

  ```
  至多一次:qos=0
  	消息发布完全依赖底层 TCP/IP 网络。会发生消息丢失或重复。这一级别可用于如下情况，环境传感器数据，丢	 失一次读记录无所谓，因为不久后还会有第二次发送。
  
  至少一次:qos=1
  	确保消息到达，但消息重复可能会发生。
  
  只有一次:qos=2 
  	确保消息到达一次。这一级别可用于如下情况，在计费系统中，消息重复或丢失会导致不正确的结果。
  ```

- 小型传输，开销很小（固定长度的头部是 2 字节），协议交换最小化，以降低网络流量；

- 使用 Last Will 和 Testament 特性通知有关各方客户端异常中断的机制。

- 一条消息可以分发给多个程序

  假设终端里有三个程序都需要采集数据：

  ```
  采集程序
     │ 发布 meter/voltage
     ▼
  MQTT Broker
     ├──→ 显示程序
     ├──→ 告警程序
     └──→ 数据存储程序
  ```

  采集程序只管发布，其他程序按主题订阅。**以后增加一个统计程序，只需要让它订阅，不必为了这个接收者修改采集程序。** 这是发布/订阅模型直接提供的能力。传统 IPC 也能实现，但你要补相应的分发逻辑：

  | 方式         | 如果多个程序都要收到同一条数据                               |
  | ------------ | ------------------------------------------------------------ |
  | 传统消息队列 | 通常需要分别投递到多个队列；共用一个队列时，一条消息被某个接收者取走，不是每个人各一份 |
  | 共享内存     | 可以共享数据，但要自己设计同步、更新通知，以及每个接收者的读取进度 |
  | socket       | 可以分别发送，或自己写一个集中分发程序                       |
  | MQTT         | Broker 根据订阅关系完成分发                                  |

  消息队列、共享内存和 Unix socket 提供了各自的基础通信机制；这些额外的广播、通知及管理策略需要由应用设计。


# Eclipse Mosquitto

## 1.mosquitto简介

![img](..\figure\cf825a6891c739b4a46c05284d131c2d.png)

Eclipse Mosquitto 是一个开源的消息代理服务器，采用 EPL/EDL 许可证授权，支持 MQTT 5.0、3.1.1 和 3.1 版本协议。Mosquitto 十分轻量，适合运行在各种设备上，从低功耗单板计算机到完整的服务器都可以使用。

MQTT 协议通过“发布/订阅”模型，提供了一种轻量级的消息传输方式。因此，它适用于物联网场景中的消息通信，例如低功耗传感器，或手机、嵌入式计算机、微控制器等移动设备之间的通信。

Mosquitto 项目还提供了一个用于实现 MQTT 客户端的 C 语言库，以及非常流行的命令行 MQTT 客户端工具 `mosquitto_pub` 和 `mosquitto_sub`。

## 2.mosquitto安装

### 1.ubuntu

1. 给安装源增加存储库

   ```bash
   sudo apt-add-repository ppa:mosquitto-dev/mosquitto-ppa
   # 更新安装源
   sudo apt update 
   ```

2. 使用apt安装mosquiito

   ```bash
   sudo apt-get install mosquitto
   ```

3. 启动重启和停止服务

   ```bash
   sudo /etc/init.d/mosquitto restart/stop/start
   
   #修改配置文件
   sudo vim /etc/mosquitto/mosquitto.conf
   ```

##  3.mosquitto配置文件

```shell
#修改配置文件
sudo vim /etc/mosquitto/mosquitto.conf
```

### 1.通用基础配置

配置写法和默认值可能随 Mosquitto 版本变化，以下内容用于理解各项用途；实际使用前以设备对应版本的手册为准。

```shell
pid_file /var/run/mosquitto/mosquitto.pid #Mosquitto 启动后，把自己的进程号（PID）写进这个文件

#服务器使用的 Linux 用户，不是客户端登录的 MQTT 账号
#以 root 启动时切换用户；以普通用户启动时，此项不起作用
#该用户需要能读取配置，并能写入日志和数据目录
user mosquitto

#是否在日志中记录客户端连接/断开事件
connection_messages true

#继续读取此目录中的 .conf 配置文件；建议放在主配置文件最后
include_dir /etc/mosquitto/conf.d
```

### 2.监听器配置

#### 1.基础监听器选项

| 配置项            | 默认值 | 说明                                                   |
| ----------------- | ------ | ------------------------------------------------------ |
| `port`            | 1883   | 服务器接收连接的端口，例如 1883；旧选项，新配置建议使用 listener |
| `bind_address`    | 无     | 服务器监听的本机 IP，不是客户端 IP；旧选项，建议使用 listener |
| `listener`        | 无     | 设置监听端口和本机 IP，例如 listener 1883 127.0.0.1 表示仅接受本机连接 |
| `max_connections` | -1     | 这个监听入口最多允许多少客户端同时连接；-1 表示不设置此限制 |
| `protocol`        | `mqtt` | 这个入口使用普通 MQTT（mqtt），还是通过 WebSocket 传输 MQTT（websockets） |
| `socket_domain`   | 自动   | 指定这个入口使用 IPv4 或 IPv6；不设置时由系统决定 |
| `http_dir`        | 无     | 使用 WebSocket 时，还可从这个目录提供网页等静态文件 |

#### 2.版本重要行为变更

> - Mosquitto 2.0 起，没有配置监听器时只接受本机连接，使用 1883 端口。
> - `port` 和 `bind_address` 是旧选项。新配置建议使用 `listener`；例如 `listener 1883 0.0.0.0` 监听本机所有 IPv4 地址。是否允许客户端登录，还要看认证配置。

**示例**：

```shell
# 默认监听器 - 绑定到所有接口的 1883 端口
listener 1883 0.0.0.0
max_connections 10000
protocol mqtt
socket_domain ipv4

# WebSocket 监听器
listener 9001 0.0.0.0
protocol websockets

# 本地管理接口（只允许本机连接）
listener 1885 127.0.0.1
```

### 3.认证与安全配置

#### 1. 基础认证

| 配置项                       | 默认值  | 说明                                                         |
| ---------------------------- | ------- | ------------------------------------------------------------ |
| `allow_anonymous`            | 随版本及监听配置变化  | true 允许不提供用户名就连接；false 要求通过已配置的认证方式；不是加密开关 |
| `password_file`              | 无      | MQTT 账号密码文件，用 mosquitto_passwd 创建；不是 Linux 的账号文件 |
| `acl_file`                   | 无      | 主题权限文件，规定哪些用户能发布或订阅哪些主题 |
| `per_listener_settings`      | `false` | false 表示各监听入口共用相关认证配置；true 表示可以分别配置。旧选项，需确认版本支持 |
| `allow_zero_length_clientid` | `true`  | true 允许 MQTT 3.1.1/5.0 客户端不填 ID，由服务器生成；false 要求自己填写。ID 不是用户名 |
| `auto_id_prefix`             | `auto-` | 自动生成客户端 ID 时使用的开头文字；新版本建议使用 listener_auto_id_prefix |
| `check_retain_source`        | `true`  | 向新订阅者发送已保存的保留消息前，是否重新检查原发布者现在还有没有发布权限 |

**allow_anonymous、账号认证和 ACL 的区别**

- `allow_anonymous true`：允许不提供用户名的客户端连接，不代表忽略客户端提供的用户名和密码。
- `password_file`：提供 MQTT 账号和密码校验数据，用来验证身份；认证插件也可以承担这个功能。
- `acl_file`：规定客户端可以发布或订阅哪些主题，不负责验证密码是否正确。
- 客户端的 `-u`、`-P` 只是提交用户名和密码，是否真的校验，取决于服务器有没有配置认证机制。这不是 Linux 登录账号。

以下讨论以普通用户名密码认证为例，假设没有其他认证插件、客户端证书或额外访问限制，且配置作用于客户端实际连接的监听器。

**情况一：允许匿名，没有账号认证**

```ini
allow_anonymous true
#没有配置 password_file，也没有其他认证机制
```

不带 `-u`、`-P` 可以连接；带上任意用户名密码也可能连接成功，因为服务器没有配置账号密码校验。不能因为客户端带了账号密码，就认为身份已经通过验证。

即使另外配置了 `acl_file`，也不会因此开始校验密码。ACL 只检查主题权限：例如 `user alice` 下面的规则不能证明客户端真的是 alice；如果用户名没有经过认证，就不能仅靠这个名字可靠地区分用户。

**情况二：允许匿名，同时启用密码文件认证**

```ini
allow_anonymous true
password_file /etc/mosquitto/passwd
acl_file /etc/mosquitto/acl
```

| 客户端提供的凭据 | 连接结果 | 连接后的主题权限 |
| --- | --- | --- |
| 不提供用户名和密码 | 允许匿名连接 | 由 ACL 的匿名用户规则等适用规则决定 |
| 提供正确的用户名和密码 | 认证成功，允许连接 | 由 ACL 中该用户的规则等适用规则决定 |
| 提供错误的用户名或密码 | 认证失败，拒绝连接，不会自动退回匿名身份 | 尚未进入主题访问阶段 |

这里拒绝错误账号密码的是认证机制，不是 ACL。连接成功后，如果没有相应的主题权限，发布或订阅仍可能被拒绝。

**情况三：所有客户端都必须通过账号密码认证**

```ini
allow_anonymous false
password_file /etc/mosquitto/passwd
acl_file /etc/mosquitto/acl
```

需要先用 `mosquitto_passwd` 准备账号文件，并确保 Mosquitto 运行用户能读取该文件。仅把 `allow_anonymous` 改为 `false`，并不会自动创建账号或启用有效的密码校验。

客户端连接示例：

```bash
mosquitto_sub -h 服务器地址 -p 1883 -t 'topic/#' -i client1 -u 'MQTT用户名' -P 'MQTT密码'
```

此命令仅演示参数，不提供传输加密。公网传输账号密码时应启用 TLS，并限制允许连接的来源；不要把允许匿名访问、没有主题权限限制的 Broker 直接开放到公网。

**总结**：认证决定能否以某个身份连接；ACL 决定该身份能访问哪些主题。允许匿名、密码校验和主题权限是三个不同的配置问题。

**实测：密码认证生效，不代表已经禁止匿名连接**

服务器同时允许匿名并启用密码文件认证时：

```ini
allow_anonymous true
password_file /etc/mosquitto/passwd
```

下面假设没有其他认证插件、客户端额外凭据配置或监听器差异，账号 device01 已创建，且相关主题权限允许测试操作。IP、账号和密码均为示例，需替换为实际值。

1. 在云服务器上提交错误密码：

   ```bash
   mosquitto_sub -h localhost -p 1883 -t 'topic/#' -i auth-test-sub -u device01 -P 'wrong-password'
   ```

   返回类似错误：

   ```text
   Connection error: Connection Refused: not authorised
   ```

   说明连接已经到达 Broker，但账号认证失败；这与 TCP 端口无法连接不是同一个问题。

2. 在云服务器上提交正确密码并保持订阅：

   ```bash
   mosquitto_sub -h localhost -p 1883 -t 'topic/#' -i auth-test-sub -u device01 -P 'correct-password' -v
   ```

   正确密码可以通过认证；之后能否收到消息，还取决于订阅权限、主题是否匹配以及是否有消息发布。

3. 在测试设备上，不带用户名密码向云服务器发布：

   ```bash
   mosquitto_pub -d -h MQTT服务器公网IP -p 1883 -t 'topic/zg' -i auth-test-pub -m 'How are you?'
   ```

   如果出现以下输出，且没有连接错误，说明连接被接受，客户端随后发送消息并主动断开：

   ```text
   Client auth-test-pub sending CONNECT
   Client auth-test-pub received CONNACK
   Client auth-test-pub sending PUBLISH ...
   Client auth-test-pub sending DISCONNECT
   ```

   这不代表密码校验失效：此连接没有提交用户名，服务器通过 allow_anonymous true 允许它匿名连接。QoS 0 的发送日志本身不证明订阅者收到了消息；如果第 2 步的订阅端显示 topic/zg 和正文，才验证了这次端到端收发。

| 测试方式 | allow_anonymous true 且启用普通密码文件认证时的预期结果 |
| --- | --- |
| 提交错误用户名或密码 | 认证失败，拒绝连接，不会自动改为匿名登录 |
| 提交正确用户名和密码 | 认证成功，允许连接，主题访问仍受 ACL 限制 |
| 不提交用户名密码 | 允许匿名连接，主题访问仍受匿名用户适用的 ACL 限制 |

**如果要求所有客户端都登录**，将实际监听器对应的配置改为：

```ini
allow_anonymous false
password_file /etc/mosquitto/passwd
```

重新加载配置后，用新连接分别测试：不带凭据应拒绝、错误凭据应拒绝、正确凭据应通过认证。不要只看已有连接，也不要只测正确密码。

排查时还要确认：localhost 指执行命令的当前机器；测试设备访问云服务器应填写云服务器地址。若不同监听器使用独立认证配置，本机入口和公网入口需要分别检查。

示例中的 -P 用于说明参数，真实密码可能留在 shell 历史或进程参数中。不要把真实凭据写进笔记；已公开的密码应及时修改，公网账号认证应配合 TLS，不能只依靠用户名密码保证传输安全。

**password_file 文件的创建和维护**

`password_file` 只指定账号文件的路径，不会自动创建文件或账号。推荐使用 `mosquitto_passwd` 工具生成和维护文件，不要自己写入明文密码。工具保存的是用户名和密码哈希校验数据，不是 Linux 用户账号。

1. 第一次创建文件并添加 MQTT 用户：

   ```bash
   sudo mosquitto_passwd -c /etc/mosquitto/passwd device01
   ```

   按提示输入两次密码。`device01` 是 MQTT 用户名，文件名 passwd 可以自行指定，但必须与配置中的路径一致。

   **注意：`-c` 会覆盖已有文件，清除其中的原有账号。只有首次创建或明确要重建整个文件时才使用。**

2. 文件已存在时，添加用户或修改密码，不要带 `-c`：

   ```bash
   #添加 device02；如果该用户已存在，就修改其密码
   sudo mosquitto_passwd /etc/mosquitto/passwd device02

   #修改 device01 的密码
   sudo mosquitto_passwd /etc/mosquitto/passwd device01
   ```

3. 删除用户：

   ```bash
   sudo mosquitto_passwd -D /etc/mosquitto/passwd device02
   ```

4. 在 Mosquitto 配置中指定账号文件，并确保服务器运行用户可以读取：

   ```ini
   allow_anonymous false
   password_file /etc/mosquitto/passwd
   ```

   如果服务器配置为 `user mosquitto`，可以根据实际权限设置该文件为 root 所有、mosquitto 组可读：

   ```bash
   sudo chown root:mosquitto /etc/mosquitto/passwd
   sudo chmod 640 /etc/mosquitto/passwd
   ```

   上级目录也必须允许 Mosquitto 用户访问；如果实际运行用户或组不同，应相应调整，不要直接照搬。

5. 修改账号文件后，让服务器重新加载：

   ```bash
   #服务支持 reload 时使用
   sudo systemctl reload mosquitto
   ```

   如果服务不支持 reload，可按前文的 SIGHUP 方法重载；也可重启服务，但会中断现有连接。重载后不会自动踢掉已经连接的客户端，验证密码修改应新建连接测试。

**总结**：`mosquitto_passwd` 负责创建、修改账号文件，`password_file` 负责指定读取路径；首次创建用 `-c`，后续维护不要用 `-c`。

#### 2 证书/TLS 配置

> 每个监听器独立配置

| 配置项                     | 说明                                    |
| -------------------------- | --------------------------------------- |
| `cafile`                   | 可信 CA 的证书文件，用于校验客户端证书，采用 PEM 格式 |
| `capath`                   | 存放可信 CA 证书的目录；更新后需运行 c_rehash 处理目录 |
| `certfile`                 | 服务器向客户端提供的证书，用于证明服务器身份 |
| `keyfile`                  | 与服务器证书配套的私钥，必须限制读取权限 |
| `tls_version`              | 限制允许的 TLS 版本，具体行为看安装版本；新部署优先 TLS 1.2/1.3，不使用 TLS 1.0/1.1 |
| `require_certificate`      | 是否要求客户端提供有效证书；默认 false，设为 true 时校验客户端证书 |
| `use_identity_as_username` | 是否把客户端证书中的 CN 字段作为 MQTT 用户名；默认 false |
| `crlfile`                  | 列出已作废证书的文件，用来拒绝这些证书 |
| `ciphers`                  | 限制 TLS 1.2 及以下可用的加密算法组合；TLS 1.3 使用单独选项 |

#### 3 预共享密钥 (PSK) 配置

| 配置项                     | 说明                                 |
| -------------------------- | ------------------------------------ |
| `psk_hint`                 | 向客户端提供选择预共享密钥的提示文字，不是密钥本身 |
| `psk_file`                 | 保存客户端标识和预先约定的密钥，格式为 identity:key，key 是十六进制密钥 |
| `use_identity_as_username` | 是否把 PSK identity 标识作为 MQTT 用户名，供后续权限检查使用         |

#### 4.认证插件配置

| 配置项                           | 默认值 | 说明                                                         |
| -------------------------------- | ------ | ------------------------------------------------------------ |
| `auth_plugin_deny_special_chars` | `true` | 认证插件检查主题权限时，拒绝用户名或客户端 ID 含 +、# 的请求，防止绕过权限检查 |

**认证模式决策树**：

```
需要认证？
├─ 否 → allow_anonymous true
└─ 是 → allow_anonymous false
    ├─ 用账号密码验证身份 → password_file
    ├─ 还要限制能访问哪些主题 → 在身份认证之外配置 acl_file
    ├─ 要求客户端提供证书 → 配置 TLS 后设置 require_certificate true
    └─ 需要在线管理账号和权限 → Dynamic Security Plugin
```

#### 5.访问控制列表 (ACL) 配置

### 4.1 ACL 文件格式

ACL 文件通过 `acl_file` 指定，支持以下语法：

| 语法                                       | 说明                                         |
| ------------------------------------------ | -------------------------------------------- |
| `topic [read\|write\|readwrite] <topic>`   | 允许订阅（read）、发布（write），或两者都允许（readwrite）                                 |
| `topic deny <topic>`                       | 禁止访问匹配的主题；先检查拒绝规则，再检查允许规则                       |
| `user <username>`                          | 后面的 topic 规则只对这个 MQTT 用户生效，不是 Linux 用户                       |
| `pattern [read\|write\|readwrite] <topic>` | 把主题中的 `%c` 替换为客户端 ID、`%u` 替换为用户名；pattern 规则对所有用户生效，不受 user 行限制 |

**匹配顺序**：先检查拒绝规则，再检查允许规则；已被拒绝的访问不会因为其他允许规则而放行。

**示例**：

```
# 匿名用户只读 sensors/ 下的数据
topic read sensors/#

# 用户 admin 全部权限
user admin
topic readwrite #

# 设备只能发布自己的数据（动态）
pattern write sensors/%u/data

# 用户 backend 可读取所有设备数据
user backend
topic read sensors/+/data
```

### 5.持久化配置

| 配置项                         | 默认值   | 说明                                                         |
| ------------------------------ | -------- | ------------------------------------------------------------ |
| `persistence`                  | `false`  | 是否把需要保留的消息和会话数据保存到文件；这不是运行日志 |
| `persistence_location`         | 当前目录 | 持久化数据的保存目录，默认文件名为 mosquitto.db |
| `autosave_interval`            | `1800`   | 定时保存间隔，单位秒；0 表示不定时保存，只在正常退出或收到 SIGUSR1 时保存 |
| `autosave_on_changes`          | `false`  | false 按时间保存；true 按订阅变化、保留消息和排队消息等累计变化次数保存，此时 autosave_interval 表示次数阈值 |
| `persistent_client_expiration` | 无       | 离线会话保留多久，例如 2d 为两天；到期未重连则删除，m 表示月 |

**持久化说明**：保存需要保留的会话、订阅关系、保留消息和排队消息等。重启后恢复已写入文件的数据；异常断电可能丢失尚未保存的内容，不是永久保存所有历史消息。

### 6.日志配置

| 配置项                | 默认值             | 说明                                                         |
| --------------------- | ------------------ | ------------------------------------------------------------ |
| `log_dest`            | `stderr`           | 运行日志输出到哪里：stderr/stdout 为标准错误/输出，syslog 交给系统日志处理，file 路径为直接写文件；可以配置多行 |
| `log_type`            | error、warning、notice、information | 选择记录哪些事件：error 错误、warning 警告、notice 提示、information 一般信息、debug 调试、subscribe 订阅、unsubscribe 退订；可配置多行，all 为全部，none 为不记录 |
| `connection_messages` | `true`             | 是否记录客户端连接和断开；不控制其他类型的日志 |
| `log_timestamp`       | `true`             | 是否在日志前加上时间，方便定位问题发生的时刻 |

**示例**：

```shell
log_dest file /var/log/mosquitto/mosquitto.log
log_dest stdout
log_type error
log_type warning
log_type notice
log_type information
connection_messages true
#记录订阅和取消订阅事件
log_type subscribe
log_type unsubscribe
```

### 7.桥接配置 (Bridging)

> 桥接让一个 Mosquitto 连接另一个 MQTT 服务器，按指定主题转发消息，不是自动把多个服务器组成集群。

| 配置项                                                       | 说明                                  |
| ------------------------------------------------------------ | ------------------------------------- |
| `connection <name>`                                          | 给这条连接远程 MQTT 服务器的桥接连接取名 |
| `address <host>[:port]`                                      | 远程 MQTT 服务器的地址，可附带端口 |
| `topic <pattern> [out\|in\|both] [qos-level] [local-prefix] [remote-prefix]` | 指定转发的主题：out 为本地发往远程，in 为远程发往本地，both 为双向；还能设置 QoS 和主题前缀                          |
| `remote_username` / `remote_password`                        | 连接远程 MQTT 服务器使用的用户名和密码                      |
| `bridge_protocol_version`                                    | 连接远程服务器使用的 MQTT 版本；如 mqttv31、mqttv311，新版本也支持 mqttv50     |
| `notifications`                                              | 是否发布桥接连接的上线、离线状态消息 |
| `cleansession`                                               | 重连时是否清除旧会话；true 清除，false 请求保留订阅和符合条件的待投递消息 |
| `start_type`                                                 | automatic 自动连接并重连；lazy 有待发送消息时连接；once 启动时连接，断开后不自动重连 |

## 八、性能与限制配置

| 配置项                     | 默认值  | 说明                                                       |
| -------------------------- | ------- | ---------------------------------------------------------- |
| `max_inflight_messages`    | `20`    | 每个客户端最多同时有多少条尚未完成确认的出站 QoS 1/2 消息；设为 1 时按顺序逐条处理 |
| `max_queued_messages`      | `100`   | 每个客户端最多排队多少条等待发送的 QoS 1/2 消息，离线或发送不及时都会用到；0 表示不限制数量 |
| `queue_qos0_messages`      | `false` | 是否也为需要保留会话的离线客户端暂存 QoS 0 消息；默认不暂存 |
| `message_size_limit`       | `0`     | 允许的消息正文大小，单位字节；0 表示不额外限制，仍受协议等限制 |
| `global_max_clients`       | `-1`    | 整个服务器最多保留多少客户端会话，包括在线和需要保留的离线会话；-1 表示不设置此限制 |
| `retry_interval`           | `20`    | 旧版本的 QoS 1/2 消息重发间隔，单位秒；现代版本已不再使用，不要直接照搬 |
| `store_clean_interval`     | `10`    | 旧版本定期清理内部消息存储的间隔，单位秒；是否支持需看安装版本 |
| `upgrade_outgoing_qos`     | `false` | 是否允许投递 QoS 高于原发布消息的 QoS；不符合标准，通常保持 false |
| `allow_duplicate_messages` | `true`  | 多个订阅都匹配同一消息时，是否允许向该客户端重复发送；旧选项，需确认版本支持 |
| `clientid_prefixes`        | 无      | 只允许客户端 ID 以指定文字开头的连接，不是用户名限制；旧选项，需确认版本支持 |

## 九、配置重载说明

修改文件后，可向 Mosquitto 发送 `SIGHUP`，让它重新读取支持重载的设置，不必停止服务器。不支持重载的设置需要重启；具体支持哪些项目，应查设备对应版本的手册。

### 支持热重载的配置项

- `acl_file`、`allow_anonymous`、`allow_zero_length_clientid`、`auto_id_prefix`
- `autosave_interval`、`autosave_on_changes`
- `connection_messages`、`global_max_clients`
- 日志、排队消息和持久化的部分选项；不是所有 log_*、persistence* 选项都能重载，需逐项确认

### 不支持热重载的配置项

- 监听器（`listener`、`port`、`bind_address`）
- TLS 的监听方式等设置；证书和私钥文件能否重新读取需查对应版本，不能一概认为都不支持
- `auth_plugin_deny_special_chars`
- `user`、`pid_file`

**重载方式**：

```
# 重载配置
sudo kill -HUP $(cat /var/run/mosquitto/mosquitto.pid)

# 上面路径必须与 pid_file 一致；或在服务支持 reload 时使用 systemd
sudo systemctl reload mosquitto
```

### 10.最小生产配置示例

先准备账号、主题权限、证书和可写目录，再按需求调整此示例，不要直接复制上线。1883 入口不加密；传输账号密码时应使用 TLS 入口。

```shell
ubuntu@VM-0-6-ubuntu:/etc/mosquitto$ cat mosquitto.conf 
# Place your local configuration in /etc/mosquitto/conf.d/
#
# A full description of the configuration file is at
# /usr/share/doc/mosquitto/examples/mosquitto.conf.gz

# ========== 基础 ==========
pid_file /run/mosquitto/mosquitto.pid
user mosquitto
connection_messages true

# ========== 监听器 ==========
# 外部访问端口
listener 1883 0.0.0.0
max_connections 10000
socket_domain ipv4

# TLS 端口
listener 8883 0.0.0.0
cafile /etc/mosquitto/certs/ca.crt
certfile /etc/mosquitto/certs/server.crt
keyfile /etc/mosquitto/certs/server.key
require_certificate false

# ========== 安全认证 ==========
allow_anonymous false
password_file /etc/mosquitto/passwd
acl_file /etc/mosquitto/acl
allow_zero_length_clientid false

# ========== 持久化 ==========
persistence true
persistence_location /var/lib/mosquitto/
autosave_interval 900

# ========== 日志 ==========
log_dest file /var/log/mosquitto/mosquitto.log
log_type error
log_type warning
log_type notice

# ========== 性能 ==========
max_inflight_messages 20
max_queued_messages 1000
message_size_limit 65536

# ========== 包含目录 ==========
include_dir /etc/mosquitto/conf.d
```

#### 2.mosquitto客户端命令

1. 使用apt安装命令行客户端

   ```bash
   sudo apt-get install mosquitto-clients
   ```

2. mosquitto_pub是推送消息，参数说明

   ```shell
   mosquitto_pub -h localhost -t "topic/test" -i "client3" -m "How are you?" -u bruce -P lan1532
   
   -h  #指定要连接的域名 默认为localhost
   -t  #指定topic
   -i  #指定客户端clientid，默认为附加进程ID的mosquitto_pub_
   -m  #消息内容
   -u  #用户名 （本地主机通信可忽略，填写的是 MQTT 服务器认可的账号，不是默认使用 Linux 的登录用户名、密码）
   -P  #用户密码
   ```

   

- -d 打印debug信息
- -f 将指定文件的内容作为发送消息的内容
- -I 指定clientId前缀
- -n 发送一个空（null）消息
- -p 连接端口号
- -q 指定QoS的值（0,1,2）
- -V 指定MQTT协议版本
- --will-payload 指定一个消息，该消息当客户端与broker意外断开连接时发出。该参数需要与--will-topic一起使用
- --will-qos Will的QoS值。该参数需要与--will-topic一起使用
- --will-retain 指定Will消息被当做一个retain消息（即消息被广播后，该消息被保留起来）。该参数需要与--will-topic一起使 用
- --will-topic 用户发送Will消息的topic

1. 使用示例

   ```bash
   mosquitto_sub -h localhost -t "topic/#" -i "client1"  -u bruce -P lan1532
   ```

2. mosquitto_sub订阅主题，参数说明

   - -c 指定客户端clean_session是否保存。
   - -d 打印debug信息
   - -h 指定要连接的域名 默认为localhost
   - -i 指定客户端clientid
   - -I 指定clientId前缀
   - -k keepalive 每隔一段时间，发PING消息通知broker，仍处于连接状态。 默认为60秒.
   - -q 指定希望接收到QoS为什么的消息 默认QoS为0
   - -R 不显示陈旧的消息
   - -t 订阅topic
   - -v 打印消息
   - --will-payload 指定一个消息，该消息当客户端与broker意外断开连接时发出。该参数需要与--will-topic一起使用
   - --will-qos Will的QoS值。该参数需要与--will-topic一起使用
   - --will-retain 指定Will消息被当做一个retain消息（即消息被广播后，该消息被保留起来）。该参数需要与--will-topic一起使>用
   - --will-topic 用户发送Will消息的topic

3. 使用示例
