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
| :--: | :--- |
| `::=` | 赋值操作符，定义类型或值 |
| `{ }` | 界定 SEQUENCE/SET/CHOICE/ENUMERATED 的成员列表，也用于值表示，还有位串的命名位表 |
| `[ ]` | 表示标签（tag），如 `[0]`、`[APPLICATION 1]`、`[PRIVATE 5]` |
| `( )` | 类型约束（如 `INTEGER (0..120)`）及值表示以及位编号 |
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

### 1.简单类型

直接规定取值集合

| 类型 | 含义 |
| :--: | :--: |
| `INTEGER` | 整数值 |
| `BOOLEAN` | 布尔值 |
| `BIT STRING` | 位序列 |
| `OCTET STRING` | 字节序列 |
| `NULL` | 空值 |
| `OBJECT IDENTIFIER` | 对象标识符 |
| `ENUMERATED` | 枚举值 |

### 2.结构类型

由多个组件构成，组件可为简单或结构类型

| 类型 | 含义 |
| :--: | :--: |
| `SEQUENCE` | 有序字段集合 |
| `SET` | 无序字段集合 |
| `SEQUENCE OF` | 同类型元素序列 |
| `SET OF` | 同类型元素集合 |
| `CHOICE` | 多选一 |

### 3. 标签类型

ASN.1 中每个类型都带一个标签（tag），由**类别 + 标签号**两部分组成。类别共四种，决定编码后标签字节的高两位。

#### 1.标签字节结构

标签在 BER 编码中是一个字节，拆三段：

```
bit 7-6   类别（class）
bit 5     构造位（constructed）：1 = 构造类型（含子项），0 = 基本类型
bit 4-0   标签号（tag number）
```

#### 2.四种类别

| 类别 | bit 7-6 | 语法写法 | 说明 | 典型例子 |
| :--: | :---: | :--: | :--: | :--- |
| **UNIVERSAL** | 00 | 直接写类型名，不带方括号 | 标准预定义的基础类型 | `INTEGER`、`OCTET STRING`、`SEQUENCE` |
| **APPLICATION** | 01 | `[APPLICATION n]` | 应用层自定义类型，跨协议通用 | `[APPLICATION 0]`（AARQ-apdu） |
| **context-specific** | 10 | `[n]`（省略类别名） | 协议内部按上下文区分，同一标签号可在不同结构中复用 | `[0]`、`[1]`（AARQ 字段） |
| **PRIVATE** | 11 | `[PRIVATE n]` | 私有扩展，厂商自定义 | `[PRIVATE 5]` |

#### 3.编码结果对照

标签字节的第 5 位是**构造位**，由类型的构造性决定：primitive（基本类型）为 0，constructed（含子项的类型，如 SEQUENCE）为 1。

| 类别 | 语法 | primitive 字节 | constructed 字节 |
| :--: | :--: | :---: | :---: |
| UNIVERSAL | 类型名 | `INTEGER` → 0x02 | `SEQUENCE` → 0x30 |
| APPLICATION | `[APPLICATION 0]` | 0x40 | 0x60 |
| context-specific | `[0]` | 0x80 | 0xA0 |
| PRIVATE | `[PRIVATE 0]` | 0xC0 | 0xE0 |

> 上表 APPLICATION / context-specific / PRIVATE 均以标签号 0 为例。标签字节 = 类别(bit7-6) + 构造位(bit5) + 标签号(bit4-0)。

#### 4.计算规则

标签字节 = 类别(bit7-6) + 构造位(bit5) + 标签号(bit4-0)。

例：`[APPLICATION 0]`（constructed）→ 01 1 00000 = `0x60` = 96。

例：`[0]`（primitive）→ 10 0 00000 = `0x80` = 128；`[0]`（constructed）→ 10 1 00000 = `0xA0` = 160。

#### 5.易混点

- `[APPLICATION 0]` 与 `[0]` 标签号相同（都是 0），但**类别不同**，编码字节不同（0x60 vs 0xA0）
- 省略类别名的 `[n]` 默认是 context-specific
- UNIVERSAL 类型（INTEGER 等）不需要也不应该写类别名，直接写类型名即可

### 4 .标签修饰符IMPLICIT 与 EXPLICIT

给类型加 `[n]` 标签时，新标签与类型自带标签（如 BIT STRING 的 UNIVERSAL 3）有两种关系，由 `IMPLICIT` / `EXPLICIT` 二选一决定。二者不仅层数不同，**外层标签的构造位也不同**。

#### 1.EXPLICIT

新标签套在内层标签**外面**。因为内层是完整的 TLV，外层标签必为 **constructed**（构造位 = 1）：

```
字段: [0] EXPLICIT BIT STRING
编码: A0  len  03  len  data
       ↑        ↑
   [0]标签      BIT STRING 标签（保留）
   (constructed=0xA0)
```

#### 2.IMPLICIT

新标签**顶替**内层标签，构造位继承被替换类型的构造性。BIT STRING 是基本类型，故 `[0]` 为 **primitive**（构造位 = 0）：

```
字段: [0] IMPLICIT BIT STRING
编码: 80  len  data
       ↑
   [0]标签（BIT STRING 的 03 被丢弃）
   (primitive=0x80)
```

#### 3.字节对比

```
EXPLICIT:  A0 <外层长度> 03 <内层长度> <内容>    两层层 TLV，[0] constructed
IMPLICIT:  80 <长度> <内容>                     一层 TLV，[0] primitive
```

#### 4.关键差异

| 项 | EXPLICIT | IMPLICIT |
| :--: | :--: | :--: |
| 层数 | 两层（外层 [0] + 内层类型） | 一层（只有 [0]） |
| 外层构造位 | constructed（`A0`） | 继承类型：基本类型 → primitive（`80`），构造类型 → constructed |
| 内层标签 | 保留（`03`） | 丢弃 |
| 编码长度 | 长 | 短 |

#### 5.选择依据

| 修饰符 | 编码 | 适用场景 |
| :--: | :--: | :--: |
| `IMPLICIT` | 省一层标签，短 | 类型明确写死，解析方已知类型，冗余可省 |
| `EXPLICIT` | 保留内层标签，长 | 类型不定（CHOICE、ANY），需靠内层标签判断实际类型 |

### 5.字段定义

```
字段名            [标签]  修饰符   类型      类型专属修饰    默认值
  │                │       │       │           │            │
protocol-version  [0]  IMPLICIT BIT STRING {version1(0)} DEFAULT {version1}
```

| 语法成分 | 示例 | 语法规则 |
| ---- | :--- | :--: |
| 字段名 | `protocol-version` | 小写开头标识符 |
| 标签 | `[0]` | `[ ]` 内放标签号，context-specific |
| 修饰符 | `IMPLICIT` | `IMPLICIT` / `EXPLICIT` 二选一 |
| 类型 | `BIT STRING` | 基础类型名 |
| 命名位表 | `{version1 (0)}` | BIT STRING 专属，给第 0 位命名 version1 |
| 默认值 | `DEFAULT {version1}` | `DEFAULT` + 值，缺省时生效 |

#### 命名位表 `{version1 (0)}`

BIT STRING 后跟 `{ }` 是**命名位表**，给位串里的位起名：

```
{ version1 (0) }
    │      │
   位名    位编号（0 = 最低位）
```

`DEFAULT {version1}` 表示缺省时该字段视为 version1（位 0 置 1）。这解释了为何报文中 `protocol-version` 可整体省略——省略即默认 version1。

# ASN.1 示例

## 1.简单类型

```ASN.1
PersonName ::= OCTET STRING
Age ::= INTEGER (0..120)
```

## 2.结构类型

```ASN.1
Person ::= SEQUENCE {
    name    PersonName,
    age     Age,
    gender  ENUMERATED { male(0), female(1) },
    address OCTET STRING OPTIONAL
}
---{}的作用：界定成员列表（类型定义时）
 alice Person ::= {
      name  "Alice",
      age   30
  }---{}的作用：值表示，具体值
```

## 3.CHOICE 类型

```ASN.1
ContactInfo ::= CHOICE {
    phoneNumber OCTET STRING,
    emailAddress OCTET STRING
}
```

## 4.SEQUENCE OF 类型

```ASN.1
PhoneNumbers ::= SEQUENCE OF OCTET STRING
```

## 5.嵌套组合

```ASN.1
Company ::= SEQUENCE {
    companyName OCTET STRING,
    employees   SEQUENCE OF Person
}
```

# DLMS 分层编码

## 1. ACSE APDU

关联控制（AARQ、AARE、RLRQ、RLRE）采用 BER 编码（ISO/IEC 8825）。字段显式携带 Tag + Length，自描述、便于调试，但报文体积较大。这部分交互频率低，冗余可接受。

## 2.xDLMS APDU

数据读写（Initiate、GET、SET、ACTION 等）采用 A-XDR（Adapted Extended Data Representation外部数据表示标准，源自 IEC 61334-6）。A-XDR 省略 Tag，按预定义顺序紧凑排列字段，编码效率高，适合窄带通信。

**"Adapted" 的含义**：A-XDR 是标准 XDR（RFC 4506，Sun 为 RPC 设计的序列化格式）的改编版，非原样照搬。主要改编点：

| 项 | 标准 XDR | A-XDR（DLMS 改编） |
| :--: | :--: | :--: |
| 对齐 | 所有数据 4 字节对齐，不足补 0 | 不强制 4 字节对齐，紧凑排列 |
| 字符串 | 长度按 4 字节倍数，含 padding | 长度字节 + 数据，不 padding |
| 浮点 | IEEE 754 标准格式 | OCTET STRING 包装，字节序有调整 |
| 应用场景 | RPC 远程调用 | 电力抄表窄带通信，省带宽优先 |

核心差异在于对齐：标准 XDR 要求每个字段从 4 字节边界开始，1 字节数据也占 4 字节（`01 00 00 00`）；A-XDR 去掉 padding，1 字节就是 1 字节（`01`）。窄带 PLC 通信字节昂贵，A-XDR 的紧凑编码正是为此改编。

## 3.层级关系

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

# COSEM 数据类型定义语法

Blue Book 中接口类属性与方法的类型定义，使用一套**独立于 ASN.1 的描述语法**（伪代码风格），而非 ASN.1 本身。

## 1.语法示例

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

## 2.与 ASN.1 的区别

| 对比项 | COSEM 定义语法 | ASN.1 |
| :--- | :--- | :--- |
| 用途 | 描述接口类属性/方法的数据类型 | 描述 ACSE APDU 等协议结构 |
| 编码规则 | 对应 A-XDR | 对应 BER（及 DER/PER 等） |
| 基本类型标签 | DLMS 自定义标签 `[0]`~`[24]` | UNIVERSAL 1~33 标准标签 |

## 3. COSEM 基本数据类型

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

# 本章总结

ASN.1 提供抽象语法与编码规则分离的机制，是 DLMS 应用层描述 ACSE APDU 结构的基础。关键要点：

- **抽象语法（ASN.1）**：描述字段结构，不涉及字节编码
- **编码规则（BER/A-XDR）**：将结构序列化为字节流
- **DLMS 分层**：ACSE 层 BER 编码（自描述），xDLMS 层 A-XDR 编码（紧凑）
- **COSEM 数据类型**：独立于 ASN.1 的 DLMS 自定义类型体系，服务于接口类属性描述
