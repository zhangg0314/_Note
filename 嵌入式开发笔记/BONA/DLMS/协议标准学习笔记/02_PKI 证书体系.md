# PKI 基本概念

**PKI（Public Key Infrastructure，公钥基础设施）** 是一套用于管理公钥信任关系的标准化体系。其核心组件包括：

| 组件 | 定义 | 说明 |
| :--: | :--: | :--: |
| **证书（Certificate）** | 将公钥与持有者身份绑定的数字文档 | 由 CA 签名，保证公钥归属可验证 |
| **CA（Certificate Authority）** | 证书颁发机构，PKI 的信任根 | 负责签发、吊销证书，验证申请者身份 |
| **证书链（Certificate Chain）** | 从根 CA 到终端实体的信任传递路径 | 通过逐级签名实现信任的层层传递 |

# 信任模型

PKI 采用层次化信任模型：

- **根 CA（Root CA）**
  信任的锚点，其证书为自签名，是整个体系的信任起点
- **中间 CA（Sub-CA / Intermediate CA）**
  由上级 CA 签发，负责代理根 CA 进行证书签发
- **终端实体证书（End-Entity Certificate）**
  由 CA 直接签发给设备或用户

验证证书时，验证方从终端证书出发，沿证书链逐级校验签名，直至到达受信任的根 CA。任一级签名校验失败，整条链即不可信。

---

# DLMS 中的 PKI 应用

## 1.主站验证 DCU 身份

DLMS 系统中，PKI 解决的核心问题是
**通信双方在未预先共享密钥的情况下，如何确认对方身份的真实性**。

**Step 1 — 建立信任根**

系统部署前，确立 Root CA 作为统一信任锚点。主站与 DCU 在出厂时均预置该根证书。根证书为自签名，其安全性依赖于部署环境中的物理保护与流程管控。

**Step 2 — 逐级签发证书**

```
Root CA（自签名，信任锚点）
    │  签发
    ▼
Sub-CA（由 Root CA 签名）
    │  签发
    ▼
DCU 证书（由 Sub-CA 签名）
    │  绑定
    ▼
DCU 公钥 + DCU 身份信息
```

**Step 3 — 身份验证流程**

1. DCU 将其证书发送给主站，证书内包含：DCU 公钥、身份信息（设备ID、厂商等）、上级 CA 的数字签名
2. 主站使用预置的根证书校验证书链：
   - 以根证书公钥验证 Sub-CA 证书的签名
   - 以 Sub-CA 公钥验证 DCU 证书的签名
   - 验证通过，主站确认该公钥确实属于该 DCU
3. 主站以 DCU 公钥验证 DCU 发出的数字签名，完成身份认证

## 2.DLMS 证书类型

DLMS 标准定义了以下密钥用途：

| 证书用途 | 密钥类型 | 适用场景 |
| :--: | :--: | :--: |
| **C(digitalSignature)** | ECDSA 签名密钥对 | 数字签名（HighECDSA 认证、固件签名） |
| **C(keyAgreement)** | ECDH 密钥协商密钥对 | 会话密钥协商（ECDH 共享密钥交换） |
| **C(TLS)** | 可选 | 基于 TLS 的传输层安全连接 |

同一设备可持有多个证书，不同用途的密钥对建议独立管理，以降低密钥泄露的影响范围。

---

# X.509 v3 证书结构

DLMS 采用 **X.509 v3** 格式定义证书结构：

```
X.509 v3 证书
├── 版本号（Version）            — v3
├── 序列号（Serial Number）      — 由颁发者分配，唯一标识
├── 签名算法（Signature Algorithm）— 如 ecdsa-with-SHA256
├── 颁发者（Issuer）             — 签发该证书的 CA 的可分辨名称（DN）
├── 有效期（Validity）           — Not Before ~ Not After
├── 主体（Subject）              — 证书持有者的可分辨名称（DN）
├── 主体公钥信息（Subject Public Key Info）
│   ├── 算法标识（Algorithm）
│   └── 公钥数据（Public Key）
├── 扩展字段（Extensions）
│   ├── 密钥用途（Key Usage）
│   ├── 扩展密钥用途（Extended Key Usage）
│   ├── 基本约束（Basic Constraints）
│   └── 证书策略（Certificate Policies）等
└── 签名值（Signature Value）    — CA 以自身私钥对上述字段的签名
```

验证时，验证方以颁发者公钥解密签名值，与本地计算的散列值比对；一致则证书未被篡改且来源可信。

---

# 证书生命周期管理

DLMS 通过以下标准方法管理证书全生命周期：

| 方法 | 操作 | 说明 |
| :--: | :--: | :--: |
| **generate_key_pair** | 生成密钥对 | 私钥存于设备内，不可导出；公钥可对外发布 |
| **generate_certificate_request** | 生成 CSR | 生成 PKCS#10 格式的证书签名请求，提交给 CA |
| **import_certificate** | 导入证书 | 将 CA 签发的证书写入设备 |
| **export_certificate** | 导出证书 | 对外提供设备证书 |
| **remove_certificate** | 移除证书 | 删除过期或吊销的证书 |
| **get_certificate_status** | 查询状态 | 检查证书是否有效、是否过期、是否被吊销 |

**典型签发流程**

```
1. HES调用generate_key_pair方法，让DCU 本地生成密钥对（私钥不离开设备）
2. HES调用generate_certificate_request方法，让DCU 生成 CSR，包含公钥及身份信息并返回CSR给HES
3. CSR 通过主站或其他通道提交至 CA
4. CA 验证申请者身份后签发证书
5. HES调用import_certificate将证书导入给 DCU
6. 后续通信中 DCU 出示证书，主站完成验证
```

---

# PKI 在 DLMS 中的价值

| 场景 | 无 PKI | 有 PKI |
| :--: | :--: | :--: |
| **新设备入网** | 需预置共享密钥，部署复杂，管理成本高 | 基于证书验证身份，无需预置对称密钥 |
| **密钥协商（ECDH）** | 无法验证对方公钥真实性，存在中间人攻击风险 | 通过证书绑定公钥与身份，防止公钥被替换 |
| **固件签名** | 对称密钥泄露后无法追溯 | 私钥由厂商持有，证书有明确有效期，支持吊销 |
| **设备更换** | 需重新配置全部密钥关系 | 仅需更换证书，吊销旧证书即可 |
| **多厂商互通** | 各厂商独立管理密钥，互信困难 | 统一信任公共根 CA，不同厂商设备可互相验证 |

**总结**：PKI 实现了公钥与身份的绑定，通过 CA 签名保证绑定的可信性，借助证书链完成信任传递。在 DLMS 体系中，PKI 为数字签名（ECDSA）和密钥协商（ECDH）提供信任基础，使无共享密钥的两端也能安全建立信任关系。
