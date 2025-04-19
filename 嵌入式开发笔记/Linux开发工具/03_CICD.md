# Gitea

# Drone

## 常见问题

### 无法解析仓库用户名

#### 问题描述

```bash
Cloning with 0 retries
Initialized empty Git repository in /app/DLMS_test/.git/
+ git fetch origin +refs/heads/master:
fatal: could not read Username for 'http://10.0.3.101:3000': terminal prompts disabled
```

#### 可能的原因及解决

##### 私有模式

此问题最常见的根本原因是源代码控制系统（GitLab、GitHub Enterprise、Gitea 等）处于私有模式，这意味着需要进行身份验证才能克隆公共存储库。默认情况下，Drone 仅在克隆私有存储库时需要进行身份验证。可以使用以下服务器设置配置 Drone 在克隆存储库时始终进行身份验证：

```yaml
DRONE_GIT_ALWAYS_AUTH=True
```

##### 从公开转为私有(已踩坑)

如果将一个仓库添加到 Drone 中，而该仓库当时是公开的，之后在源代码管理系统中将其更改为私有。Drone 可能不会意识到这一变更。这可能会导致此问题，因为我们需要进行身份验证才能克隆私有 Git 仓库。如果 Drone 不知道该仓库现在是私有的，那么在尝试克隆时它将不会进行身份验证，从而导致克隆失败。通常可以通过重新同步账户来解决此问题，但在某些情况下，如果问题仍然存在，则可能需要手动更新数据库。

##### 权限已被撤销

如果使用用户账户激活了代码库，然后删除了该用户账户或撤销了 Drone 对该用户账户的访问权限，系统将无法克隆该代码库。此问题可通过停用代码库然后重新激活来解决。重新激活代码库的用户将被分配所有权。

##### 无效的用户密码

由于克隆存储库不需要向Drone提供用户名和密码，Drone会自动使用oauth2令牌进行身份验证。
如果需要覆盖默认身份验证行为并提供静态克隆凭据，要添加仔细检查用户名和密码的配置，并确保所添加的用户具有克隆存储库的权限。

```yaml
DRONE_GIT_USERNAME=<username>
DRONE_GIT_PASSWORD=<password>
```

### YAML文件格式错误

#### 问题描述1

```bash

```

