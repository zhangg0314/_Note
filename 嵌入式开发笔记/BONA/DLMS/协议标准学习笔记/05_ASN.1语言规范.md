# ASN.1 概述

ASN.1（Abstract Syntax Notation One，抽象语法记法一）是 ITU-T X.680 系列建议定义的国际标准，用于描述数据结构。它解决的核心问题是：不同系统之间如何无歧义地交换结构化数据。

关键性质：

- **抽象语法与编码规则解耦**——ASN.1 只描述"数据长什么样"，不规定"怎么编码成字节"
- 编码规则独立成体系（BER、DER、PER 等），同一种抽象语法可选多种编码规则
- 最初由 ITU 为电信协议设计，后被 ISO 采纳为国际标准

# ASN.1 的主要应用

| 领域 | 典型协议/标准 |
| :--: | :--: |
| 网络管理 | SNMP（SMI 结构与 MIB 定义） |
| 目录服务 | LDAP、X.500 |
| 电信信令 | SS7、GSM |
| 安全协议 | X.509 数字证书、SSL/TLS、PKCS |
| 金融 | ISO 20022 |
| 多媒体 | H.323、H.264 |
| 智能卡 | 电子护照、支付卡 |

# 编码规则

ASN.1 抽象语法通过不同的编码规则序列化为字节流。常见编码规则：

| 编码规则 | 全称 | 特点 | 典型用途 |
| :--: | :--: | :--: | :--: |
| **BER** | Basic Encoding Rules | TLV（Tag-Length-Value）自描述，灵活但有冗余 | 通用传输 |
| **DER** | Distinguished Encoding Rules | BER 子集，消除不确定性，编码唯一 | X.509 证书、数字签名 |
| **CER** | Canonical Encoding Rules | BER 子集，使用不定长编码，适合流式传输 | 大体积数据的流式处理 |
| **PER** | Packed Encoding Rules | 紧凑编码，省带宽，但解析复杂 | 带宽受限场景 |
| **XER** | XML Encoding Rules | 编码为 XML，可读性好但冗长 | XML 系统互操作 |
| **OER** | Octet Encoding Rules | 高效且易解析 | 紧凑快速解析场景 |
| **JER** | JSON Encoding Rules | 编码为 JSON | JSON 系统互操作 |

> BER 的 TLV 结构：每个数据项由 **类型标签（Tag）+ 长度（Length）+ 内容（Value）** 三段组成。

# ASN.1 语法规范

## 1.文字约定

- 区分大小写
- 空格无实义，仅作分隔
- 注释以 `--` 开头，到 `--` 或行尾结束
- 标识符由大小写字母、数字、横线组成
- 内部关键字全大写（如 `INTEGER`、`SEQUENCE`）
- 用户定义的类型名和模块名以大写字母开头
- 变量名（数据对象）以小写字母开头

## 2.关键字

| 关键字 | 含义 |
| :--: | :--: |
| `INTEGER` | 整型数据 |
| `BOOLEAN` | 布尔值（TRUE / FALSE） |
| `BIT STRING` | 位串 |
| `OCTET STRING` | 字节串 |
| `NULL` | 空值 |
| `OBJECT IDENTIFIER` | 对象标识符，唯一标识对象 |
| `SEQUENCE` | 序列类型，含多个有序字段 |
| `SET` | 集合类型，元素顺序无关 |
| `CHOICE` | 选择类型，多选一 |
| `ENUMERATED` | 枚举类型，取预定义常量 |
| `SEQUENCE OF` | 同类型元素的序列 |
| `SET OF` | 同类型元素的集合，顺序无关 |
| `ANY` | 任意类型（已弃用，被信息对象类替代） |
| `OPTIONAL` | 标记可选字段 |
| `DEFAULT` | 字段默认值 |
| `EXPLICIT` | 显式标签（保留内层标签） |
| `IMPLICIT` | 隐式标签（替换内层标签） |
| `DEFINITIONS` | 模块定义起始关键字 |
| `END` | 模块定义结束关键字 |
| `BEGIN` | 与 `DEFINITIONS` 配套的模块体起始 |

## 3.符号

| 符号 | 含义 |
| :--- | :--- |
| `::=` | 赋值操作符，定义类型或值 |
| `{ }` | 界定 SEQUENCE/SET/CHOICE/ENUMERATED 的成员列表，也用于值表示 |
| `[ ]` | 标签（tag），如 `[0]`、`[APPLICATION 1]`、`[PRIVATE 5]` |
| `( )` | 类型约束（如 `INTEGER (0..120)`）及值表示 |
| `,` | 分隔列表中的多个元素 |
| `\|` | 分隔 CHOICE/ENUMERATED 中的可选项 |
| `..` | 约束中的范围符号（如 `0..120`） |
| `...` | 扩展标记，表示类型可向后扩展 |
| `--` | 注释 |
| `.` | OBJECT IDENTIFIER 的弧分隔符（如 `2.16.756`） |
| `'...'B` | 二进制位串值（如 `'101'B`） |
| `'...'H` | 十六进制字符串值（如 `'FF'H`） |

> 说明：ASN.1 **不使用**分号 `;` 作为语句终止符，也**不使用** `< >` 定义类型。`%x`、`%d`、`%o`、`%b` 等前缀非 ASN.1 语法，常见误解来源是 printf 格式符与 DLMS A-XDR 记法的混淆。

## 4.数据类型

**简单类型**（直接规定取值集合）

| 类型 | 含义 |
| :--- | :--- |
| `INTEGER` | 整数值 |
| `BOOLEAN` | 布尔值 |
| `BIT STRING` | 位序列 |
| `OCTET STRING` | 字节序列 |
| `NULL` | 空值 |
| `OBJECT IDENTIFIER` | 对象标识符 |
| `ENUMERATED` | 枚举值 |

**结构类型**（由多个组件构成，组件可为简单或结构类型）：

| 类型 | 含义 |
| :--- | :--- |
| `SEQUENCE` | 有序字段集合 |
| `SET` | 无序字段集合 |
| `SEQUENCE OF` | 同类型元素序列 |
| `SET OF` | 同类型元素集合 |
| `CHOICE` | 多选一 |

**标签类型**：ASN.1 支持四类标签——`UNIVERSAL`（通用）、`APPLICATION`（应用）、`context-specific`（上下文专用，用 `[n]` 表示）、`PRIVATE`（私有）。

# ASN.1 示例

## 1.简单类型

```
PersonName ::= OCTET STRING
Age ::= INTEGER (0..120)
```

## 2.结构类型

```
Person ::= SEQUENCE {
    name    PersonName,
    age     Age,
    gender  ENUMERATED { male(0), female(1) },
    address OCTET STRING OPTIONAL
}
```

## 3.CHOICE 类型

```
ContactInfo ::= CHOICE {
    phoneNumber OCTET STRING,
    emailAddress OCTET STRING
}
```

## 4.SEQUENCE OF 类型

```
PhoneNumbers ::= SEQUENCE OF OCTET STRING
```

## 5.嵌套组合

```
Company ::= SEQUENCE {
    companyName OCTET STRING,
    employees   SEQUENCE OF Person
}
```

## 六、DLMS 中的编码分层

DLMS 协议在应用层使用**两种编码规则**，分层配合。

### 6.1 ACSE APDU 用 BER

关联控制（AARQ、AARE、RLRQ、RLRE）采用 BER 编码（ISO/IEC 8825）。字段显式携带 Tag + Length，自描述、便于调试，但报文体积较大。这部分交互频率低，冗余可接受。

### 6.2 xDLMS APDU 用 A-XDR

数据读写（Initiate、GET、SET、ACTION 等）采用 A-XDR（Adapted Extended Data Representation外部数据表示标准，源自 IEC 61334-6）。A-XDR 省略 Tag，按预定义顺序紧凑排列字段，编码效率高，适合窄带通信。

**"Adapted" 的含义**：A-XDR 是标准 XDR（RFC 4506，Sun 为 RPC 设计的序列化格式）的改编版，非原样照搬。主要改编点：

| 项 | 标准 XDR | A-XDR（DLMS 改编） |
| :--- | :--- | :--- |
| 对齐 | 所有数据 4 字节对齐，不足补 0 | 不强制 4 字节对齐，紧凑排列 |
| 字符串 | 长度按 4 字节倍数，含 padding | 长度字节 + 数据，不 padding |
| 浮点 | IEEE 754 标准格式 | OCTET STRING 包装，字节序有调整 |
| 应用场景 | RPC 远程调用 | 电力抄表窄带通信，省带宽优先 |

核心差异在于对齐：标准 XDR 要求每个字段从 4 字节边界开始，1 字节数据也占 4 字节（`01 00 00 00`）；A-XDR 去掉 padding，1 字节就是 1 字节（`01`）。窄带 PLC 通信字节昂贵，A-XDR 的紧凑编码正是为此改编。

### 6.3 层级关系

```
AARQ APDU（BER 编码）
├── protocol-version            （BIT STRING，BER）
├── application-context-name    （OID，BER）
├── called-AP-title             （BER）
├── ...
└── user-information            （BER 包装）
    └── OCTET STRING            （BER 包装）
        └── xDLMS InitiateRequest APDU（A-XDR 编码，无 Tag 紧凑排列）
```

**一句话**：ACSE 层（建立/释放关联）用 BER，xDLMS 层（数据操作）用 A-XDR，二者在同一条报文中嵌套共存。

## 七、AARQ 报文解析示例

报文（Wrapper + AARQ）：

```
00 01 00 10 00 01 00 38 60 36 A1 09 06 07 60 85 74 05 08 01 01
A2 03 02 01 44 A3 03 02 01 44 A4 03 02 01 00 A5 03 02 01 00
A8 03 02 01 00 BE 10 04 0E 01 00 00 00 06 5F 1F 04 00 62 FE DF FF FF
```

### 7.1 Wrapper Header（8 字节）

| 字节 | 值 | 字段 | 含义 |
| :--- | :--- | :--- | :--- |
| 00-01 | `00 01` | Version | Wrapper 版本 = 1 |
| 02-03 | `00 10` | Source wPort | 源 wPort = 16 |
| 04-05 | `00 01` | Destination wPort | 目的 wPort = 1 |
| 06-07 | `00 38` | Data Length | 后续 APDU 长度 = 56 字节 |

### 7.2 AARQ APDU（BER 编码，56 字节）

**外层标签与总长度**：

| 字节 | 值 | 含义 |
| :--- | :--- | :--- |
| 08 | `60` | AARQ-apdu 标签，`[APPLICATION 0]` |
| 09 | `36` | 内容总长度 = 54 字节 |

**application-context-name `[1]`**：

| 字节 | 值 | 含义 |
| :--- | :--- | :--- |
| 0A | `A1` | 显式标签 `[1]`，context-specific constructed |
| 0B | `09` | 长度 = 9 字节 |
| 0C | `06` | OID 标签（UNIVERSAL 6） |
| 0D | `07` | OID 长度 = 7 字节 |
| 0E-14 | `60 85 74 05 08 01 01` | OID 值 |

OID 解码 `60 85 74 05 08 01 01`：

- 首字节 `60` = 0x60，首弧 = 96 ÷ 40 = 2，余数 96 − 2×40 = 16 → **2.16**
- `85 74`：`0x85 & 0x7F = 5`（continuation 置位），续 `0x74 = 116` → 5×128 + 116 = **756**
- `05` → 5，`08` → 8，`01` → 1，`01` → 1

最终 OID = **2.16.756.5.8.1.1**，即 application-context = Logical Name referencing, no ciphering（逻辑名引用、无加密）。

**called-AP-title `[2]`**：

| 字节 | 值 | 含义 |
| :--- | :--- | :--- |
| 15 | `A2` | 显式标签 `[2]` |
| 16 | `03` | 长度 = 3 字节 |
| 17 | `02` | INTEGER 标签（UNIVERSAL 2） |
| 18 | `01` | 长度 = 1 字节 |
| 19 | `44` | 值 = 68（十进制） |

> 注意：`02` 是 **INTEGER** 标签，不是 OID（OID 标签是 `06`）。因此 `A2 03 02 01 44` 内部是 **INTEGER 68**，而非 OID。

**其余字段**（结构同上述，均为 `[n]` 显式标签 + INTEGER）：

| 字段 | 字节序列 | 值 |
| :--- | :--- | :--- |
| called-AE-qualifier `[3]` | `A3 03 02 01 44` | INTEGER 68 |
| called-AP-invocation-identifier `[4]` | `A4 03 02 01 00` | INTEGER 0 |
| called-AE-invocation-identifier `[5]` | `A5 03 02 01 00` | INTEGER 0 |
| calling-AP-invocation-identifier `[8]` | `A8 03 02 01 00` | INTEGER 0 |

> 说明：`protocol-version [0]` 字段在本报文中**未出现**。其 ASN.1 定义为 `DEFAULT {version1}`，缺省时默认取 version1，故合法省略。

**user-information `[30]`**：

| 字节 | 值 | 含义 |
| :--- | :--- | :--- |
| 2D | `BE` | 显式标签 `[30]`，context-specific constructed |
| 2E | `10` | 长度 = 16 字节 |
| 2F | `04` | OCTET STRING 标签（UNIVERSAL 4） |
| 30 | `0E` | OCTET STRING 长度 = 14 字节 |
| 31-44 | `01 00 00 00 06 5F 1F 04 00 62 FE DF FF FF` | xDLMS InitiateRequest（A-XDR 编码） |

### 7.3 xDLMS InitiateRequest（A-XDR 编码，14 字节）

A-XDR 无 Tag，按字段顺序紧凑排列：

| 字节 | 值 | 字段 | 含义 |
| :--- | :--- | :--- | :--- |
| 31 | `01` | dedicated-key 长度 | 长度 1，内容 `00`（无实际专用密钥） |
| 32 | `00` | response-allowed | FALSE（不允许响应） |
| 33 | `00` | proposed-quality-of-service | 不使用 |
| 34 | `00` | proposed-dlms-version-number | DLMS 版本 = 0 |
| 35 | `06` | proposed-conformance 长度 | 6 字节 |
| 36-41 | `5F 1F 04 00 62 FE` | proposed-conformance | 6 字节一致性位图（3 个 2 字节块） |
| 42-44 | `DF FF FF` | client-max-receive-pdu-size | 客户端最大接收 PDU 大小 |

### 7.4 报文汇总

```
Wrapper Header (8B)   → 00 01 00 10 00 01 00 38
└── AARQ APDU (56B)   → 60 36 ...
    ├── application-context-name  → A1 09 06 07 60 85 74 05 08 01 01
    │                               (Logical_Name_Referencing_No_Ciphering)
    ├── called-AP-title            → A2 03 02 01 44   (INTEGER 68)
    ├── called-AE-qualifier        → A3 03 02 01 44   (INTEGER 68)
    ├── called-AP-invocation-id    → A4 03 02 01 00   (INTEGER 0)
    ├── called-AE-invocation-id    → A5 03 02 01 00   (INTEGER 0)
    ├── calling-AP-invocation-id   → A8 03 02 01 00   (INTEGER 0)
    └── user-information           → BE 10 04 0E ...
        └── InitiateRequest (A-XDR, 14B)
            ├── dedicated-key      → 01 00 (无专用密钥)
            ├── response-allowed   → 00 (FALSE)
            ├── proposed-QoS       → 00
            ├── proposed-version   → 00
            ├── conformance        → 06 + 5F 1F 04 00 62 FE
            └── max-receive-pdu    → DF FF FF
```

## 八、COSEM 数据类型定义语法

Blue Book 中接口类属性与方法的类型定义，使用一套**独立于 ASN.1 的描述语法**（伪代码风格），而非 ASN.1 本身。

### 8.1 语法示例

```
value ::= array {
    structure {
        meter_device_id  visible_string,
        meter_data       structure
    }
}
```

| 语法元素 | 含义 |
| :--- | :--- |
| `value ::=` | 定义名为 value 的类型 |
| `array { ... }` | 数组，元素为花括号内定义的类型 |
| `structure { ... }` | 结构体，含多个字段 |
| `meter_device_id visible_string` | 字段名 + 类型（`visible_string` 为基本类型） |
| `meter_data structure` | 字段名 + 嵌套结构体类型 |

类型别名定义：

```
PIID ::= unsigned          -- 定义 PIID 为 unsigned（Unsigned8）类型
```

### 8.2 与 ASN.1 的区别

| 对比项 | COSEM 定义语法 | ASN.1 |
| :--- | :--- | :--- |
| 用途 | 描述接口类属性/方法的数据类型 | 描述 ACSE APDU 等协议结构 |
| 编码规则 | 对应 A-XDR | 对应 BER（及 DER/PER 等） |
| 基本类型标签 | DLMS 自定义标签 `[0]`~`[24]` | UNIVERSAL 1~33 标准标签 |

### 8.3 COSEM 基本数据类型（Common Data Types）

| 类型名 | 描述 | 值范围 | A-XDR Tag |
| :--- | :--- | :--- | :--- |
| `null-data` | 空数据 | — | [0] |
| `boolean` | 布尔值 | TRUE / FALSE | [3] |
| `bit-string` | 位串 | — | [4] |
| `double-long` | 32 位有符号整数 | −2³¹ ~ 2³¹−1 | [5] |
| `double-long-unsigned` | 32 位无符号整数 | 0 ~ 2³²−1 | [6] |
| `octet-string` | 字节序列 | — | [9] |
| `visible-string` | ASCII 可见字符串 | — | [10] |
| `utf8-string` | UTF-8 字符串 | — | [12] |
| `bcd` | BCD 编码 | — | [13] |
| `integer` | 8 位有符号整数 | −128 ~ 127 | [15] |
| `long` | 16 位有符号整数 | −32768 ~ 32767 | [16] |
| `unsigned` | 8 位无符号整数 | 0 ~ 255 | [17] |
| `long-unsigned` | 16 位无符号整数 | 0 ~ 65535 | [18] |
| `long64` | 64 位有符号整数 | −2⁶³ ~ 2⁶³−1 | [20] |
| `long64-unsigned` | 64 位无符号整数 | 0 ~ 2⁶⁴−1 | [21] |
| `enum` | 枚举 | 0 ~ 255 | [22] |
| `float32` | 32 位浮点 | — | [23] |
| `float64` | 64 位浮点 | — | [24] |

## 九、总结

ASN.1 提供抽象语法与编码规则分离的机制，是 DLMS 应用层描述 ACSE APDU 结构的基础。关键要点：

- **抽象语法（ASN.1）**：描述字段结构，不涉及字节编码
- **编码规则（BER/A-XDR）**：将结构序列化为字节流
- **DLMS 分层**：ACSE 层 BER 编码（自描述），xDLMS 层 A-XDR 编码（紧凑）
- **COSEM 数据类型**：独立于 ASN.1 的 DLMS 自定义类型体系，服务于接口类属性描述
