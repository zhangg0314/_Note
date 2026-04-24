# Git介绍

​	Git 是一个开源的分布式版本控制系统（ Distributed Version Control System，简称 `DVCS` ），用于高效地管理项目版本。它能够有效、高速地处理从小到大的项目版本管理，追踪项目从开始到结束的整个过程。

​	版本控制（ Version Control ）是软件配置管理的一项内容，主要对程序、文档等项目制品的版本变更进行管理。它允许开发者记录、追踪和管理代码或其他文件的历史变化，以便在需要时能够恢复到以前的版本或查看历史记录。

​	版本控制是团队协作开发的桥梁，有助于多人协作同步进行大型项目开发。版本控制系统是团队协作开发的桥梁，它通过分支管理、**自动合并（无需传统的人工拿着两个温度看哪里添加哪里删除来修改合并）和冲突解决**（如果多个开发人员修改了同一个文件，版本控制系统会标记冲突，开发人员可以手动解决冲突。）、历史记录和追溯、代码审查等功能，支持多人协作同步进行大型项目的开发。它不仅提高了开发效率，还确保了代码质量和项目的稳定性。

# 集中式vs分布式

## 1.集中式版本控制系统

​	版本库集中存放在中央服务器。最大的毛病就是必须联网才能工作。

## 2.分布式版本控制系统

​	没有“中央服务器”，每个人的电脑上都是一个完整的版本库。在自己电脑上改了文件A，同事也在他的电脑上改了文件A，这时，只需把各自的修改推送给对方，就可以互相看到对方的修改了。

​	安全性要高很多，因为每个人电脑里都有完整的版本库，某一个人的电脑坏掉了不要紧，随便从其他人那里复制一个就可以了，这是因为每个人都需要在本地创建版本库，并且把修改提交到版本库了才可以推送即push前都要先commit，所以相当于强制性要求了每个人在推送前都要在本地备份一样。

​	而集中式版本控制系统的中央服务器要是出了问题，所有人都没法干活了。分布式版本控制系统通常也有一台充当“中央服务器”的电脑，但这个服务器的作用仅仅是用来方便“交换”大家的修改，没有它大家也一样干活，只是交换修改不方便而已。

# Git的作用和目的

1. **版本控制**
   Git允许开发者跟踪代码的版本历史，可以回溯到任何时刻的代码状态。这对于调试、修复错误以及恢复代码都非常有用。
2. **分支管理**
   Git支持创建多个分支，使得团队成员可以并行地开发不同的功能和修复不同的bug。<u>每个分支都可以独立地进行开发和测试</u>，最后再将分支合并到主分支上。这种分支策略提高了开发效率和灵活性。
3. **合作开发**
   Git提供了远程仓库的功能，使得团队成员可以共享和合并代码。通过Git，开发者可以轻松地将代码推送到共享仓库，并进行同步，从而促进了团队协作和代码共享。
4. **撤销和回退**
   Git允许开发者撤销之前的提交，回退到之前的版本。这对于修复错误或不需要的更改非常有用，保证了代码的稳定性和可维护性。
5. **快速部署**
   Git可以轻松部署代码到生产环境。通过创建发布分支或打标签，开发者可以准确地控制代码的部署版本，提高了部署的效率和准确性。
6. **高度可靠性和安全性**
   Git的分布式架构意味着每个开发者都有一份完整的代码库，即使服务器出现故障，开发者仍然可以继续工作。此外，Git使用哈希算法来标识每个版本，确保每个版本的唯一性和完整性，提高了代码的安全性和可靠性。

# 仓库各种状态解析

## 1.工作区（Working Directory）

​	就是我们敲命令，改代码的当前目录，我们平时更新版本什么的，都是在这里完成的，可以理解成是在这里工作的。

## 2.版本库（Repository）

​	工作区有一个隐藏目录 `.git`，这个不算工作区，而是 Git 的版本库。可以认为有.git隐藏目录的目录算一个仓库，大仓库可以嵌套小仓库。

Git 的版本库里存了很多东西，其中最重要的就是称为 `stage`（或者叫 index）的暂存区，还有 Git 为我们自动创建[***<u>不是真正的创建，要先进行一次commit操作（进行一次提交操作），才会真正建立master分支。这是因为分支的指针要指向提交的,只有进行了提交，才有指针指向该分支，才算是真正的建立了分支，成为一个有效的对象。</u>***]的第一个分支 `master`，以及指向 `master`的一个指针叫 `HEAD`。

红色代表新增文件或者被修改文件，绿色代表位于暂存区的文件。

# Git基本命令

## 1.创建版本库

```bash
#初始化仓库，在当前也就是敲git init命令的目录下多一个.git目录，提交的内容是指整个当前目录的所有内容
git init 
```

## 2.全局配置

```bash
#命令行输入user.name/user.email后，每一次Git提交都会使用这些信息，它们会写入到每一次提交中，用于标识身份,表明此次是由谁提交的。

git config --global user.name "username1"
git config --global user.email "xxx@163.com"

#查看配置信息

git config --list

#修改配置信息
git config --global --replace-all user.name "username2"
git config --global --replace-all user.email "xxx@qq.com"
```

## 3.查看库状态

```bash
#查看本地库状态，此时文件是untracked files，位于分支 master
git status
	尚无提交
	未跟踪的文件:
  	（使用 "git add <文件>..." 以包含要提交的内容）
        learngit/
	提交为空，但是存在尚未跟踪的文件（使用 "git add" 建立跟踪）
```

**未跟踪文件的产生主要有以下几种原因**：

1. 新添加的文件。
2. 生成的文件（如编译输出、临时文件等）。
3. 临时文件和日志文件。
4. 从远程仓库克隆后添加的文件。
5. 忽略文件的更改后重新生成的文件。

## 4.添加到本地暂存区

```shell
git add  <文件名1> <文件名2> ...
```

## 5.提交到本地

```shell
git commit -m "此次提交的信息"
```

## 6.修改提交信息

- 修改最近一次提交信息

  ```bash
  #这将打开默认的文本编辑器（通常是vi或nano），显示当前的提交信息
  git commit --amend
  ```

- 修改历史提交信息

  ```bash
  git rebase -i HEAD~N #慎用，可能会导致其他人拉代码的时候出问题
  #其中 N 是想要回溯的提交数量。例如，如果想修改最近3次提交，可以使用HEAD~3 
  ```

## 7.比较差异

```shell
git diff readme.txt 


diff --git a/learngit/readme1.txt b/learngit/readme1.txt  #进行比较的是,a版本的readme1(即变动前)和b版本的readme1(即变动后).
index e69de29..b4de394 100644#表示两个版本的git哈希值(index区域的e69de29对象,与工作目录区域的b4de394对象进行比较),最后的六位数字是对象的模式(普通文件,644权限).
--- a/learngit/readme1.txt#表示进行比较的两个文件."-"表示变动前的版本,"+++"表示变动后的版本
+++ b/learngit/readme1.txt
@@ -0,0 +1 @@ -0,0 #表示旧版本的0到0行，+1表示新版本的0-1行
+11
```

## 8.撤销修改

```shell
git checkout -- <被修改的文件名> 
#或者
git restore <file>
```

## 9.查看日志

```bash
#查看提交日志
git log

#查看历史git命令，寻找版本号
git reflog
```

## 10.版本回退

```shell
#回退到上一次commit，HEAD指向是当前活跃分支的最近一次提交。
git reset --hard HEAD^   
	[option]
		--hard #撤销最近一次提交，并丢弃所有未提交的更改
		--mixed #默认选项，撤销最近一次提交，取消暂存区的更改，但保留工作区的更改
		--soft  #撤销最近一次提交，但保留暂存区的更改

#根据版本号回退到版本
git reset --hard 1094a   
```

## 11.切换提交

切换版本通常是指切换到不同的提交（commit）、分支（branch）或标签（tag）

```shell
git checkout <commit-hash>

#注意：在这种情况下，Git 会进入“分离头指针”（detached HEAD）状态。在这种状态下，你的工作区将指向一个特定的提交，而不是一个分支。如果你在这个状态下进行提交，这些提交将不会关联到任何分支。如果你希望将这些提交关联到一个分支，可以创建一个新的分支：

git checkout -b <new-branch-name>
```

## 12.从分离头指针切换回分支

当前处于分离头指针状态（例如，通过  git checkout <commit-hash>   切换到了某个特定的提交），然后执行以下命令切换回一个分支：

```shell
git switch main
#或
git checkout main
```

# Git分支命令

## 1.查看分支

```bash
# 查看所有本地分支
git branch

# 查看所有远程分支
git branch -r

# 查看所有分支（本地和远程）
git branch -a
```

## 2.操作分支

```bash
#查看指定commit的id的文件的本次提交到哪些分支上了，即哪些分支包含了此处提交
git branch -a --contains cfc6773b07b6392144e124ee594aeaa6939e6bd7

#新增本地分支
git branch 分支名

#切换分支
git checkout 目的分支名

#创建并切换到跟踪分支
git switch -c <本地分支名> origin/<远程分支名>
# 示例：创建并切换到跟踪 origin/develop 的分支
git switch -c develop origin/develop

#在本地分支之间切换
git switch  目的分支名

#删除本地分支
git branch -D 分支名

#合并目标分支到当前所在分支
git merge 目标分支名
```

## 3.合并分支冲突解决

1. 修改冲突的文件

2. 把修改好的冲突文件手动添加到暂存区
3. 继续添加到版本库

# Git远程管理

## 1.绑定本地和远程仓库

```shell
#克隆下来的仓库在本地的别名，省略默认为origin
git remote add <仓库别名> git@git:192.168.65.113/term/ModularizationTerminal.git
```

## 2.修改远程仓库绑定

```bash
git remote set-url <remote-name> <new-url>
```

## 3.使用git clone一键配置

​	在一个未初始化的目录中执行git clone命令时，Git会自动创建一个新的.git目录，并将克隆的仓库的内容复制到该目录中。

​	此外，Git还会自动为克隆的仓库设置一个名为origin的远程仓库，指向克隆的源地址。
具体来说，git clone会执行以下操作：

1. 创建一个新的目录，如果没有指定目录名，Git会使用仓库的名称。
2. 在该目录中创建一个.git 目录，包含所有的版本控制信息。
3. 将远程仓库的所有文件和历史记录复制到本地。
4. 设置origin作为默认的远程仓库名称，指向克隆的源地址。
   因此，我们不需要在克隆之前手动执行git init，git clone会自动处理这些步骤。

## 4.克隆指定分支

```shell
#指定分支克隆，省略则为main或者master。
git clone -b <分支名> https://github.com/user/repo.git
```

## 5.查看远程仓库

```shell
git remote -v

origin  http://192.168.65.113/term/ModularizationTerminal.git (fetch)
origin  http://192.168.65.113/term/ModularizationTerminal.git (push)
zhangg  ssh://git@192.168.65.113:term/ModularizationTerminal.git (fetch)
zhangg  ssh://git@192.168.65.113:term/ModularizationTerminal.git (push)
```

## 6.从远程拉取

```shell
#从远程的仓库的指定分支下载代码到本地的当前分支上
git fetch <远程仓库名> <远程仓库分支>

git pull <远程仓库名> <远程分支名> == git fetch + git merge
```

## 7.推送到远程

推送步骤：提交本地修改——>pull合并远程代码到本地——>解决合并冲突——>push推送到远程

```bash
#如果远程仓库分支省略，则默认为与本地分支同名。
git push <远程仓库名> <本地分支名>[:远程分支名] 
```

## 8.撤销远程推送

```bash
git reset --hard [commit_id] 使本地仓库回退到指定提交。
#找到要回退到的提交ID。 同样使用 git log 查看。执行reset操作。 

git push --force 
#将本地更改强制推送到远程。这一步操作比较危险，因为它会覆盖远程仓库的历史即修改远程仓库的提交记录，所以在团队项目中要慎用。
```

# Git定义别名

```shell
#全局配置文件，执行git --config命令的值都保存在此文件中
sudo vi ~/.gitconfig

[alias]
	ls = log
	st = status
	... ... 
	自定义名 = 命令名
```

# 设置不跟踪文件

## 1.使用   .gitignore   文件

1. 创建.gitignore（项目根目录下）

2. 添加忽略规则

   ```bash
   filename.txt  #忽略单个文件
   foldername/	  #忽略某个文件夹
   foldername/*  #忽略某个文件夹下的所有文件
   *.log
   *.tmp		 #忽略特定类型的文件
   foldername/filename.txt#忽略特定文件夹中的特定文件
   foldername/*.log #忽略特定文件夹中的特定类型文件
   ```

3. 保存并提交   .gitignore   文件

   ```bash
   git add .gitignore
   git commit -m "Add .gitignore file"
   ```

## 2.官方配置文件

[gitignore/VisualStudio.gitignore at main · github/gitignore · GitHub](https://github.com/github/gitignore/blob/main/VisualStudio.gitignore)

# 添加 git 子模块

[Git 自动添加 git 子模块(.gitmodules)|极客教程](https://geek-docs.com/git/git-questions/74_git_adding_git_submodules_automatically_gitmodules.html)

# 设置文件属性

[git配置文件—— .gitattributes - qiqi715 - 博客园](https://www.cnblogs.com/qiqi715/p/9286468.html)

## 1.LF 和 CRLF 

它们是**电脑里表示「换一行」的隐藏符号**，

本质就是：**不同操作系统，对「怎么标记换行」的规矩不一样**。

------

## 2. 两个符号的区别

### ①` LF` = `\n`

**全称：Line Feed（换行）**

👉 **Linux 系统、Mac 电脑** 默认用的换行符

👉 只用 **1 个符号** 表示换行

### ② `CRLF `= `\r\n`

**全称：Carriage Return + Line Feed（回车 + 换行）**

👉 **Windows 电脑** 默认用的换行符

👉 用 **2 个符号** 表示换行

------

## 3. 为什么 Git 要管这个

因为：

- 用 **Windows** → 文件默认 `CRLF`
- 服务器 / 别人用 **Linux/Mac** → 文件默认` LF`

Git 会自动转换换行符，

结果就是：

**文件内容明明没改，却显示 modified（被修改）**

这就是 **换行符冲突**。

------

## 4. 该怎么配置

### 1.windows

```bash
git config --global core.autocrlf true

#意思：提交时自动转 LF，拉取时自动转 CRLF → 不报错
```

### 2.Mac/Linux

```bash
git config --global core.autocrlf input
```
