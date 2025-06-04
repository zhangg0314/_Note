

# SQLite3介绍

- SQLite3是一种轻量级的关系型数据库管理系统（RDBMS），它以跨平台、零配置、服务器-less的方式存储数据。
- SQLite3不像其他常见的数据库管理系统，如MySQL或PostgreSQL那样需要一个独立的服务器进程，在应用程序内部直接操作文件来进行数据存储和读取。
- SQLite3非常适合于嵌入式设备和单机应用程序等场景，因为它不需要占用太多资源，也允许在不同的平台上运行。 
- SQLite3支持大多数SQL语法，并且还提供了一些高级功能，如触发器、存储过程等。

# SQLite3优势

- 跨平台性：SQLite3可以在多种操作系统和编程语言下使用，包括Windows、Linux、macOS、iOS、Android等。
- 零配置：SQLite3的特点之一是不要求任何服务器或网络配置。只需将数据库文件嵌入应用程序即可轻松地访问数据
- 体积小：SQLite3的核心库非常小，通常只有几百KB，因此非常适合在资源受限或空间受限的系统中使用
- 支持SQL：SQLite3支持大多数标准SQL查询语言，使用户能够使用大多数传统数据库管理任务
- ACID兼容：SQLite3支持ACID（原子性、一致性、隔离性和持久性）事务处理，确保数据始终处于一致状态
- 高可靠性：SQLite3对于频繁读取和少量更新的场景，表现出色。由于其自动记录更改，以防止损坏和数据丢失
- 强大的API：SQLite3提供了一个简单易用的C语言API来操作数据库，同时也提供了大量的接口和工具
- 可扩展性：SQLite3允许用户创建自己的函数和存储过程，从而增加了其灵活性和可扩展性

# SQLite3下载安装

- 下载SQLite3二进制文件：可以从SQLite网站的下载页面（[SQLite Download Page](https://www.sqlite.org/download.html)）下载适用于相应系统的SQLite3二进制文件

- 安装SQLite3二进制文件：

  - 对于Windows，只需运行安装程序并按照提示进行即可

   - 对于Linux，您需要使用包管理器来安装SQLite3软件包

     ```shell
     sudo apt-get update
     sudo apt-get install sqlite3
     ```

- 验证SQLite3是否已正确安装：

  - 在命令行终端上输入sqlite3命令并回车
  - 如果成功安装，则会进入SQLite3的命令行客户端

- 可选地，安装SQLite3 GUI工具：如果您需要一个GUI工具来管理SQLite3数据库，则可以使用一些第三方工具

  - SQLiteStudio
  - DBeaver
  - 这些工具可免费下载和使用

注意：安装sqlite3和安装sqlite3依赖库不是一个概念，安装sqlite3依赖库，需要执行`sudo apt-get install libsqlite3-dev`

# SQLite3命令

| 命令                                | 说明                                           |
| ----------------------------------- | ---------------------------------------------- |
| .archive ...                        | 管理SQL归档文件                                |
| .auth ON&#124;OFF                   | 显示授权回调函数                               |
| .backup ?DB? FILE                   | 将数据库DB（默认为“main”）备份到FILE           |
| .bail on&#124;off                   | 在错误发生后停止。默认为OFF                    |
| .binary on&#124;off                 | 打开或关闭二进制输出。默认为OFF                |
| .cd DIRECTORY                       | 将工作目录更改为DIRECTORY                      |
| .changes on&#124;off                | 显示SQL更改的行数                              |
| .check GLOB                         | 如果自从.testcase以来的输出不匹配，则失败      |
| .clone NEWDB                        | 从现有数据库克隆数据到NEWDB中                  |
| .databases                          | 列出附加数据库的名称和文件                     |
| .dbconfig ?op? ?val?                | 列出或更改sqlite3_db_config()选项              |
| .dbinfo ?DB?                        | 显示有关数据库的状态信息                       |
| .dump ?TABLE? ...                   | 将所有数据库内容呈现为SQL                      |
| .echo on&#124;off                   | 打开或关闭命令回显                             |
| .eqp on&#124;off&#124;full&#124;... | 启用或禁用自动EXPLAIN QUERY PLAN               |
| .excel                              | 在电子表格中显示下一个命令的输出               |
| .exit ?CODE?                        | 以返回代码CODE退出此程序                       |
| .expert                             | 实验性功能。为查询建议索引                     |
| .explain ?on&#124;off&#124;auto?    | 更改EXPLAIN格式模式。默认值：auto              |
| .filectrl CMD ...                   | 运行各种sqlite3_file_control()操作             |
| .fullschema ?--indent?              | 显示模式和sqlite_stat表的内容                  |
| .headers on&#124;off                | 打开或关闭标题的显示                           |
| .help ?-all? ?PATTERN?              | 显示有关PATTERN的帮助文本                      |
| .import FILE TABLE                  | 将数据从FILE导入TABLE                          |
| .imposter INDEX TABLE               | 在索引INDEX上创建仿冒表TABLE                   |
| .indexes ?TABLE?                    | 显示索引的名称                                 |
| .limit ?LIMIT? ?VAL?                | 显示或更改SQLITE_LIMIT的值                     |
| .lint OPTIONS                       | 报告潜在的模式问题。                           |
| .load FILE ?ENTRY?                  | 加载扩展库                                     |
| .log FILE&#124;off                  | 打开或关闭日志记录。文件可以是stderr/stdout    |
| .mode colum                         | 列对齐模式                                     |
| .nullvalue STRING                   | 在NULL值的位置使用字符串STRING                 |
| .once (-e&#124;-x&#124;FILE)        | 仅将下一个SQL命令的输出发送到FILE              |
| .open ?OPTIONS? ?FILE?              | 关闭现有数据库并重新打开FILE                   |
| .output ?FILE?                      | 将输出发送到FILE，如果省略FILE，则发送到stdout |
| .parameter CMD ...                  | 管理SQL参数绑定                                |
| .print STRING...                    | 打印文本字符串STRING                           |
| .progress N                         | 在每个N操作码后调用进度处理程序                |
| .prompt MAIN CONTINUE               | 替换标准提示符                                 |
| .quit                               | 退出SQLite3的客户端                            |
| .read FILE                          | 从FILE读取输入                                 |
| .recover                            | 尝试从损坏的数据库中恢复尽可能多的数据。       |
| .restore ?DB? FILE                  | 从FILE还原数据库DB（默认为“main”）的内容       |
| .save FILE                          | 将内存中的数据库写入FILE                       |
| .scanstats on&#124;off              | 打开或关闭sqlite3_stmt_scanstatus()指标        |
| .schema ?PATTERN?                   | 显示与PATTERN匹配的CREATE语句                  |
| .selftest ?OPTIONS?                 | 运行在SELFTEST表中定义的测试                   |
| .separator COL ?ROW?                | 更改列和行分隔符                               |
| .session ?NAME? CMD ...             | 创建或控制会话                                 |
| .sha3sum ...                        | 计算数据库内容的SHA3哈希值                     |
| .shell CMD ARGS...                  | 在系统Shell中运行CMD ARGS...                   |
| .show                               | 显示各种设置的当前值                           |
| .stats ?on&#124;off?                | 显示统计信息或打开/关闭统计信息                |
| .system CMD ARGS...                 | 在系统Shell中运行CMD ARGS...                   |
| .tables ?TABLE?                     | 列出LIKE模式为TABLE的表名                      |
| .testcase NAME                      | 开始将输出重定向到“testcase-out.txt”           |
| .testctrl CMD ...                   | 运行各种sqlite3_test_control()操作             |
| .timeout MS                         | 尝试以MS毫秒为间隔打开已锁定表                 |
| .timer on&#124;off                  | 打开或关闭SQL计时器                            |
| .trace ?OPTIONS?                    | 在每个SQL语句执行时输出                        |
| .vfsinfo ?AUX?                      | 有关顶级VFS的信息                              |
| .vfslist                            | 列出所有可用的VFS                              |
| .vfsname ?AUX?                      | 打印VFS堆栈的名称                              |
| .width NUM1 NUM2 ...                | 设置“ column”模式的列宽度                      |
|                                     |                                                |

# SQL语句

## 1.创建表格

```sqlite
CREATE TABLE table_name (
  column_1 data_type constraints,
  column_2 data_type constraints,
  ...,
  column_n data_type constraints
);
```

## 2.删除表格

```sqlite
DROP TABLE table_name;
```

## 3.查询数据

```sqlite
SELECT column_1, column_2, ..., column_n 
FROM table_name;
```

## 4.插入数据

```sqlite
INSERT INTO table_name (column_1, column_2, ..., column_n)
VALUES (value_1, value_2, ..., value_n);
```

## 5.更新数据

```sqlite
UPDATE table_name
SET column_1 = value_1, column_2 = value_2, ..., column_n = value_n
WHERE condition;
```

## 6.删除数据

```sqlite
DELETE FROM table_name WHERE condition;
```

# SQLite3使用示例（命令行 ）

以下是使用SQLite3创建一个名为“students”的学生信息表并插入三个学生信息的示例：

1. 打开`shell`命令行终端。 

2. 连接到SQLite3数据库文件。 

   ```bash
   $ sqlite3 mydatabase.db
   ```

3. 创建一个名为“students”的学生信息表。该表将包含以下列：ID，姓名，年龄，性别和成绩。

   ```sqlite
   sqlite> CREATE TABLE students (
      ...> ID INTERGER PRIMARY KEY NOT NULL,
      ...> name TEXT NOT NULL,
      ...> age INTEGER NOT NULL,
      ...> gender TEXT NOT NULL,
      ...> score REAL NOT NULL);
   ```

备注：`sqlite>`和`   ...>`是sqlite3的命令行提示符


```
sqlite> .table
students
```

如果没有创建成功，或者表被删除，将会什么也不显示，例如此时执行DROP语句，将看不到表了

```
sqlite> DROP TABLE students;
sqlite> .table
sqlite>
```

重复上面的`CREATE TABLE`（为了放便大家复制，粘贴后面把`sqlite3>`, `   ...>`都省略了）

```
CREATE TABLE students (
ID INTERGER PRIMARY KEY NOT NULL,
name TEXT NOT NULL,
age INTEGER NOT NULL,
gender TEXT NOT NULL,
score REAL NOT NULL);
```

4. 插入三个学生信息。例如：

```
INSERT INTO students (ID, name, age, gender, score)
   ...> values(1,'张三', 18, '男', 90.0);
INSERT INTO students (ID, name, age, gender, score)
values(2,'子涵', 18, '女', 88.5);
INSERT INTO students (ID, name, age, gender, score)
values(3, '张伟', 19, '男', 82.5);
```

这将向学生信息表中添加三条记录。每条记录都包含姓名、年龄、性别和分数等列的值。由于我们使用了自增长的主键列ID，因此每个记录都将在插入时自动被赋予一个唯一的ID值。

5. 确认学生信息已成功插入。可以使用`SELECT`语句来检索学生信息表中的所有行。例如：

```
select * FROM students;
1|张三|18|男|90.0
2|子涵|18|女|88.5
3|张伟|19|男|82.5
```

如果嫌弃这种界面，可以执行.headers on和.mode column，再执行SELECT语句，就好看了，如下图：

```sql
sqlite> .headers on
sqlite> .mode column
sqlite> select * FROM students;
ID          name        age         gender      score     
----------  ----------  ----------  ----------  ----------
1           张三          18          男           90.0      
2           子涵          18          女           88.5      
3           张伟          19          男           82.5  
```

6. 退出SQLite shell或关闭终端会话。

```
sqlite> .exit
```

# sqlite3编程

## 1.打开数据库

```c
/*打开一个SQLite数据库文件*/
int sqlite3_open(
    const char *filename,   /*数据库文件的文件名，如果为 ":memory:" 则表示创建内存中数据库*/
    sqlite3 **ppDb          /*返回指向数据库连接句柄的指针*/
);
```

这个函数使用指定的数据库文件名（或 `:memory:`）打开一个新的数据库连接，并且把连接句柄返回给调用者。如果连接成功，将会返回 `SQLITE_OK` 编码（0），否则将会返回其他错误代码。需要注意的是，SQLite 数据库文件不存在时将会自动创建。如果 `filename` 参数为 `NULL` ，该函数返回 `SQLITE_MISUSE` （错用 SQLite 调用）错误。`ppDb` 参数是一个二级指针，它将存储一个指向打开的数据库连接的指针，该指针需要在后续的 SQLite 操作中使用。例如，以下是在 C 语言中打开一个 SQLite 数据库的示例代码：

```c
#include <stdio.h>
#include <sqlite3.h>
int main() {
    sqlite3 *db;
	/*打开数据库*/
	int rc = sqlite3_open("test.db", &db);
	if (rc == SQLITE_OK) {
		printf("已成功打开数据库\n");
		sqlite3_close(db); // 关闭数据库连接
	} else {
		fprintf(stderr, "无法打开数据库: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 1;
	}
	return 0;
}
```

## 2.关闭数据库

```c
/*关闭指定数据库连接*/
int sqlite3_close(sqlite3*);
```

该函数接收一个被打开的数据库连接指针，如果成功关闭返回 `SQLITE_OK` 编码（0），否则返回其他错误代码。在使用完SQLite数据库连接之后，应该释放它以避免资源泄漏和内存占用。调用 `sqlite3_close()` 函数将关闭数据库连接，并释放所有相关的资源，包括已编译的语句等。如果在关闭连接之前还有未完成的事务，则会自动回滚这些事务。例如，以下是在 C 语言中关闭 SQLite 数据库连接的示例代码：

```c
#include <sqlite3.h>
int main() {
    sqlite3 *db;
    int rc = sqlite3_open("test.db", &db);
    // 执行一些数据库操作
    // ...
    rc = sqlite3_close(db);
    if (rc == SQLITE_OK) {
      printf("Closed database successfully\n");
  } else {
      fprintf(stderr, "Can't close database: %s\n", sqlite3_errmsg(db));
  }
    return 0;
}
```

## 3.sqlite3_exec()

### 1.函数介绍

 此函数用于在 SQLite 数据库连接上执行一条或多条 SQL 语句，并调用一个回调函数处理执行结果。该函数的原型如下：

```c
int sqlite3_exec(
    sqlite3* db,                               /* 执行 SQL 命令的数据库连接 */
    const char *sql,                           /* 待执行的 SQL 命令 */
    int (*callback)(void*,int,char**,char**),  /* 在执行命令时的回调函数 */
    void *,                                    /* 作为第一个参数传递给回调函数的指针 */
    char **errmsg                              /* 用于存储错误消息的指针 */
);
```

该函数接收一个打开的数据库连接 `sqlite3*`，待执行的 SQL 命令 `sql`，以及一个回调函数 `callback`，可以选择性地传递一个指向用户数据的指针作为回调函数的第一个参数，该句话的意思是，回调函数的参数列表（签名）决定了该函数在被调用时应接收哪些参数，并指定了它们的数据类型和顺序。回调函数必须符合以下格式：

```c
int (*callback)(void*, int, char**, char**);
```

- 第一个参数 `void*`，是使用者传递给 `sqlite3_exec()` 调用的 `void*` 参数。
- 第二个参数 `int`，是查询结果所返回的列数。
- 第三个参数 `char**`，是包含每个结果集元素值的字符串数组。一个字符数组代表一行，数组每个元素代表一个字符串，该字符串内容为该行每一列的值。
- 第四个参数 `char**`，是包含每个结果集元素的列名称的字符串数组。这通常会在 SELECT 语句中返回。

回调函数的返回值应为整数，并且通常全部返回0表示执行成功。如果需要提前终止查询或在回调函数过程中发现错误，可以返回非零值
`errmsg` 是用于保存 `sqlite3_exec()` 返回的错误消息的指针。
当函数成功执行 SQL 命令时，将会返回 `SQLITE_OK` 编码（0），否则将会返回其他错误代码。如果在执行命令时发生错误，则 `errmsg` 将被设置为一个非空值，其中包含有关错误的详细信息。如果 `errmsg` 未被设置，则表示该函数执行成功。

### 2.创建表

以下是一个使用 `sqlite3_exec()` 函数执行 SQL 命令的示例代码：

```c
#include <sqlite3.h>
#include <stdio.h>
int main(int argc, char* argv[]) {
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	/*打开数据库*/
	rc = sqlite3_open("test.db", &db);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 1;
	} else {
		fprintf(stdout, "Opened database successfully\n");
	}
	/*创建表的SQL语句*/
	char *sql = "CREATE TABLE IF NOT EXISTS COMPANY(" //表已经存在则打开表，表不存在则创建。
		"ID INT PRIMARY KEY     NOT NULL,"
		"NAME           TEXT    NOT NULL,"
		"AGE            INT     NOT NULL);";

	/*执行SQL语句*/
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	} else {
		printf("Table created successfully\n");
	}
	/*关闭数据库文件*/
	sqlite3_close(db);
	return 0;
}
```

上述代码中，首先调用sqlite3_open()函数打开一个名为"test.db"的数据库连接，然后使用**CREATE TABLE语句**创建一张名为"COMPANY"的表格。该表格包含三列，分别是"id"、"name"和"age"**,**
		其中，id列被定义为主键（PRIMARY KEY），且不能为NULL。
		接下来，通过调用sqlite3_exec()函数执行SQL语句，
		将该语句传递给SQLite引擎进行解析和执行。
		如果执行成功，则输出"Table created successfully"，
		否则输出具体的错误消息。

### 3.插入数据

创建好表格以后我们就可以向其添加数据了：

```c
#include <sqlite3.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

	/*打开数据库*/
    rc = sqlite3_open("test.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    } else {
        fprintf(stdout, "Opened database successfully\n");
    }

	/*创建表的SQL语句*/
    char *sql = "INSERT INTO COMPANY (ID,NAME,AGE) VALUES (1, '张三', 32);"
        "INSERT INTO COMPANY (ID,NAME,AGE) VALUES (2, '李四', 33);"
    	"INSERT INTO COMPANY (ID,NAME,AGE) VALUES (3, '王五', 30);"
    	"INSERT INTO COMPANY (ID,NAME,AGE) VALUES (4, '王博', 32);"
        "INSERT INTO COMPANY (ID,NAME,AGE) VALUES (5, '李为', 33);"
    	"INSERT INTO COMPANY (ID,NAME,AGE) VALUES (6, '赵倩', 30);";
    
    //类似于sprintf
    char sno[STRLEN],sname[STRLEN],sex[STRLEN],age[STRLEN],sdept[STRLEN];
    sql = sqlite3_mprintf("INSERT INTO student" 
						"(Sno,Sname,Sex,Age,Sdept) VALUES ('%s','%s','%s','%s','%s');",sno,sname,sex,age,sdept);


	/*执行SQL语句*/
    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Records created successfully\n");
    }

	/*关闭数据库文件*/
    sqlite3_close(db);
    return 0;
}
```

在上述代码中，需要先调用sqlite3_open()函数打开数据库连接。然后，使用INSERT INTO语句将一条记录插入到"COMPANY"表格中。该记录包含三个字段，分别是"id"、"name"和"age"，对应的值分别为1、'张三'和32。最后，通过sqlite3_exec()函数执行SQL语句，并根据返回值判断操作是否成功。

### 4.查询数据

```c
#include <sqlite3.h>
#include <stdio.h>

// 回调函数
int callback(void *data, int argc, char **argv, char **azColName) {
    int i;
    printf("callback:\n");
    for(i = 0; i < argc; i++) {
       printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
    printf("\n");

    return 0;
}

int main () {
    sqlite3 *db;
    char *zErrMsg = 0; // 存储错误消息的指针
    int rc;

    rc = sqlite3_open("test.db", &db);
    if (rc != SQLITE_OK) {
       fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
       sqlite3_close(db);
       return 1;
   }

    const char* sql = "SELECT * from COMPANY";
    rc = sqlite3_exec(db, sql, callback, NULL, &zErrMsg);//callback表单有几行就会被调用几次

    if (rc != SQLITE_OK) {
       fprintf(stderr, "SQL error: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
   } else {
       printf("Operation done successfully\n");
   }

    sqlite3_close(db);
    return rc;
}
```

在此示例中，我们定义了一个回调函数 `callback()`，该函数用于处理 `sqlite3_exec()` 执行结果。接下来，我们打开一个名为 "test.db" 的 SQLite 数据库连接，并执行 SELECT 命令。最后，我们通过检查 `rc` 和 `zErrMsg` 的返回值，判断查询是否成功并处理执行结果。

### 5.void *data的用法

void *data是sqlite3_exec()函数的第四个参数，它是一个用户定义的指针类型，提供了一个通用的方法来传递额外的数据给回调函数。sqlite3_exec()函数的高级用法
示例代码：

```c
#include <sqlite3.h>
#include <stdio.h>

// 回调函数
int callback(void *data, int argc, char **argv, char **azColName) {
	int i;
	if(argv[0][0] == '1')
		printf("%s\n", (char *)data);
	for(i = 0; i < argc; i++) {
		printf("%s\t", argv[i] ? argv[i] : "NULL");
	}
	printf("\n");

	return 0;
}

int main () {
	sqlite3 *db;
	char *zErrMsg = 0; // 存储错误消息的指针
	int rc;

	rc = sqlite3_open("test.db", &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 1;
	}

	char *str = "ID\t名字\t年龄";
	char *sql = "SELECT * from COMPANY";
	rc = sqlite3_exec(db, sql, callback, str, &zErrMsg);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	} else {
		printf("Operation done successfully\n");
	}

	sqlite3_close(db);
	return rc;
}
```

# SQL高效执行方式

## 1.sqlite3_prepare_v2()和sqlite3_exec()的区别

### 1. 功能

- **`sqlite3_prepare_v2()`**：
  - **功能**：将 SQL 语句编译成一个可执行的虚拟机代码，返回一个 `sqlite3_stmt` 类型的句柄。这个句柄可以用于后续的执行操作。
  - **用途**：通常用于需要多次执行相同 SQL 语句的场景，例如参数化查询。
  - **优点**：可以提高性能，因为编译后的 SQL 语句可以多次执行，避免了重复编译。
  - **缺点**：使用相对复杂，需要手动管理句柄的生命周期（如调用 `sqlite3_finalize()` 释放资源）。
- **`sqlite3_exec()`**：
  - **功能**：直接执行一条或多条 SQL 语句，不需要预先编译。
  - **用途**：适用于简单的 SQL 操作，如创建表、插入单条数据等。
  - **优点**：使用简单，不需要手动管理句柄。
  - **缺点**：性能较低，因为每次调用都会重新编译 SQL 语句。不支持参数化查询，容易受到 SQL 注入攻击。

### 2. 参数

- **`sqlite3_prepare_v2()`**：
  - **参数**：
    - `sqlite3 *db`：数据库连接。
    - `const char *zSql`：SQL 语句。
    - `int nByte`：SQL 语句的长度（字节）。如果为负值，会自动计算长度。
    - `sqlite3_stmt **ppStmt`：编译后的 SQL 语句句柄。
    - `const char **pzTail`：指向剩余未处理的 SQL 语句。
  - **返回值**：返回状态码，如 `SQLITE_OK`、`SQLITE_ERROR` 等。
- **`sqlite3_exec()`**：
  - **参数**：
    - `sqlite3 *db`：数据库连接。
    - `const char *sql`：SQL 语句。
    - `int (*callback)(void*,int,char**,char**)`：回调函数，用于处理查询结果。
    - `void *arg`：回调函数的参数。
    - `char **errmsg`：存储错误信息。
  - **返回值**：返回状态码，如 `SQLITE_OK`、`SQLITE_ERROR` 等。

### 3. 使用场景

- **`sqlite3_prepare_v2()`**：
  - **参数化查询**：适用于需要多次执行相同 SQL 语句，但参数不同的场景。
  - **性能优化**：适用于需要多次执行相同 SQL 语句的场景，可以避免重复编译。
  - **复杂查询**：适用于需要逐步处理查询结果的场景，如分页查询。
- **`sqlite3_exec()`**：
  - **简单操作**：适用于简单的 SQL 操作，如创建表、插入单条数据等。
  - **一次性操作**：适用于不需要多次执行的 SQL 语句。
  - **批量操作**：可以一次性执行多条 SQL 语句。

## 2.高效方式

为解决sqlite3_exec函数执行效率低的问题，就出现了其它更加高效的解决方式：**将sqlite3_exec的功能进行分解，由多个函数共同完成**。这就是本篇要介绍的：

- **sqlite3_prepare_v2()**函数：实现对sql语句(模板)的解析和编译，生成了可以被执行的 sql语句实例
- **sqlite3_stmt()**数据结构：可以理解为一种“准备语句对象”，它可以结合变量使用，进而实现相同操作的循环
- **sqlite3_bind_** *函数：用于绑定赋值变量
- **sqlite3_step()** 函数：用于执行sql语句

相比较使用sqlite3_exec函数，现在这种方式，sql语句的解析和编译只执行了一次，而sqlite3_step执行多次，整体的效率势必大大提升。

## 3.**sqlite3_stmt()**数据结构

`sqlite3_stmt` 是一个抽象类型，其具体内容在 SQLite 的实现中并未完全暴露给用户。它是一个指向内部数据结构的指针，用于表示编译后的 SQL 语句

## 4.sqlite3_prepare_v2()

### 1.函数介绍

该函数实现对sql语句(模板)的解析和编译，生成了可以被执行的sql语句实例

```c
int sqlite3_prepare_v2(
  sqlite3 *db,            /* 数据库连接 */
  const char *zSql,       /* SQL 语句 */
  int nByte,              /* SQL 语句的长度（字节） */
  sqlite3_stmt **ppStmt,  /* 准备好的 SQL 语句句柄 */
  const char **pzTail     /* 指向剩余未处理的 SQL 语句 */
);
//参数：
	-db：sqlite数据库
	-zSql：要执行的sql语句（可以包含未赋值的变量）
	-nByte：sql语句的(字符串的)长度，SQL 语句的长度（字节）。如果为负值，则函数会自动计算字符串的长度（直到遇到第一个 \0）。
	-ppStmt：解析编译出的sql语句实例,指向 sqlite3_stmt 类型的指针，用于存储编译后的 SQL 语句句柄。
	-pzTail：指向剩余未处理的 SQL 语句的指针。如果 SQL 语句中包含多条语句，pzTail 会指向第一条语句之后的内容。	
//返回值：
    -SQLITE_OK：成功准备 SQL 语句。
    -SQLITE_ERROR：SQL 语句有语法错误。
    -SQLITE_NOMEM：内存分配失败。
    -SQLITE_MISUSE：API 使用错误。
```

### 2.示例代码

```c
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    const char *sql;
    int rc;

    // 打开数据库
    rc = sqlite3_open("example.db", &db);
    if (rc) {
        fprintf(stderr, "无法打开数据库: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 创建一个表
    sql = "CREATE TABLE IF NOT EXISTS contacts (id INTEGER PRIMARY KEY, name TEXT, phone TEXT)";
    rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 插入数据
    sql = "INSERT INTO contacts (name, phone) VALUES ('Alice', '1234567890')";
    rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 准备查询语句
    sql = "SELECT * FROM contacts";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "无法准备 SQL 语句: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 执行查询并处理结果
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("ID: %d, Name: %s, Phone: %s\n",
               sqlite3_column_int(stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_text(stmt, 2));
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "查询失败: %s\n", sqlite3_errmsg(db));
    }

    // 清理
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 0;
}
```

## 5.sqlite3_step()

### 1.函数介绍

```c
int sqlite3_step(sqlite3_stmt *pStmt);
//参数说明
	-pStmt：指向 sqlite3_stmt 类型的指针，表示预编译的 SQL 语句句柄。这个句柄是通过 sqlite3_prepare_v2() 或其他类似的函数生成的。
//返回值
	-SQLITE_ROW：表示查询返回了一行数据。可以通过 sqlite3_column_* 函数获取这一行的数据。
	-SQLITE_DONE：表示 SQL 语句执行完成，没有更多的数据返回。
	-SQLITE_ERROR：表示执行过程中发生错误。
	-SQLITE_MISUSE：表示 API 使用错误，例如在不适当的状态下调用了 sqlite3_step()。
	-其他错误代码：表示其他类型的错误。
```

### 2.示例

```c
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    const char *sql;
    int rc;

    // 打开数据库
    rc = sqlite3_open("example.db", &db);
    if (rc) {
        fprintf(stderr, "无法打开数据库: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 创建一个表
    sql = "CREATE TABLE IF NOT EXISTS contacts (id INTEGER PRIMARY KEY, name TEXT, phone TEXT)";
    rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 插入数据
    sql = "INSERT INTO contacts (name, phone) VALUES ('Alice', '1234567890')";
    rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 准备查询语句
    sql = "SELECT * FROM contacts";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "无法准备 SQL 语句: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 执行查询并处理结果
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("ID: %d, Name: %s, Phone: %s\n",
               sqlite3_column_int(stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_text(stmt, 2));
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "查询失败: %s\n", sqlite3_errmsg(db));
    }

    // 清理
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 0;
}
```

## 6.sqlite3_bind_ *

在 SQLite 中，`sqlite3_bind_*` 是一系列函数，用于将参数绑定到预编译的 SQL 语句中。这些函数通常与 `sqlite3_prepare_v2()` 一起使用，用于参数化查询，从而提高安全性和性能。参数化查询可以有效防止 SQL 注入攻击，并且可以多次执行相同的 SQL 语句，只需改变参数值。

### 1.函数列表

SQLite 提供了多种 `sqlite3_bind_*` 函数，用于绑定不同类型的参数。以下是一些常见的函数：

1. **`sqlite3_bind_blob()`**：绑定二进制数据。
2. **`sqlite3_bind_double()`**：绑定双精度浮点数。
3. **`sqlite3_bind_int()`**：绑定 32 位整数。
4. **`sqlite3_bind_int64()`**：绑定 64 位整数。
5. **`sqlite3_bind_null()`**：绑定 NULL 值。
6. **`sqlite3_bind_text()`**：绑定文本字符串。
7. **`sqlite3_bind_text16()`**：绑定 UTF-16 编码的文本字符串。
8. **`sqlite3_bind_value()`**：绑定任意类型的值（通过 `sqlite3_value` 类型）。

### 2.函数介绍

```c
//以 sqlite3_bind_text() 为例，其函数原型如下：
int sqlite3_bind_text(
  sqlite3_stmt *stmt,       /* 预编译的 SQL 语句句柄 */
  int index,                /* 参数的索引（从 1 开始） */
  const char *value,        /* 要绑定的文本值 */
  int n,                    /* 文本值的长度（字节） */
  void (*xDel)(void*)      /* 释放绑定值的回调函数 */
);

//参数说明
	-stmt：指向 sqlite3_stmt 类型的指针，表示预编译的 SQL 语句句柄。
	-index：参数的索引，从 1 开始。例如，? 在 SQL 语句中的位置。
	-value：要绑定的值。
	-n：值的长度（字节）。如果为负值，会自动计算长度。
	-xDel：释放绑定值的回调函数。如果绑定的值是动态分配的，可以指定一个回调函数来释放内存。
        	通常使用 SQLITE_STATIC 或 SQLITE_TRANSIENT。
        	-SQLITE_STATIC：表示绑定的值是静态的，SQLite 可以直接使用这个值，而不需要复制它。通常用于绑定全局变量或常量。
            -SQLITE_TRANSIENT：表示绑定的值是临时的，SQLite 需要复制这个值，以确保在 SQL 语句执行时值仍然有效。通常用于绑定局部变量或动态分配的内存。
//返回值
	-SQLITE_OK：成功绑定参数。
	-SQLITE_ERROR：绑定失败。
	-SQLITE_TOOBIG：绑定的值太大。
	-SQLITE_MISUSE：API 使用错误。
```

### 3.示例

```c
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    const char *sql;
    int rc;

    // 打开数据库
    rc = sqlite3_open("example.db", &db);
    if (rc) {
        fprintf(stderr, "无法打开数据库: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 创建表
    sql = "CREATE TABLE IF NOT EXISTS contacts (id INTEGER PRIMARY KEY, name TEXT, phone TEXT)";
    rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 插入数据
    sql = "INSERT INTO contacts (name, phone) VALUES (?, ?)";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "无法准备 SQL 语句: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, "Alice", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, "1234567890", -1, SQLITE_STATIC);

    // 执行插入操作
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "插入失败: %s\n", sqlite3_errmsg(db));
    }

    // 清理
    sqlite3_finalize(stmt);

    // 查询数据
    sql = "SELECT * FROM contacts";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "无法准备 SQL 语句: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 执行查询并处理结果
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("ID: %d, Name: %s, Phone: %s\n",
               sqlite3_column_int(stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_text(stmt, 2));
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "查询失败: %s\n", sqlite3_errmsg(db));
    }

    // 清理
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 0;
}
```

## 7.sqlite3_finalize()

`sqlite3_finalize()` 是 SQLite 中的一个重要函数，用于销毁一个预编译的 SQL 语句（`sqlite3_stmt`）对象。这个函数通常在完成 SQL 语句的执行后调用，以释放与该语句相关的所有资源。

### 1.函数介绍

```c
int sqlite3_finalize(sqlite3_stmt *pStmt);
//参数说明
	-pStmt：指向 sqlite3_stmt 类型的指针，表示预编译的 SQL 语句句柄。这个句柄是通过 sqlite3_prepare_v2() 或其他类似的函数生成的。
//返回值
	-SQLITE_OK：成功销毁了 SQL 语句句柄。
	-SQLITE_ERROR：销毁失败。
	-其他错误代码：表示其他类型的错误。
```

### 2.示例

```c
if (rc != SQLITE_DONE) {
        fprintf(stderr, "查询失败: %s\n", sqlite3_errmsg(db));
    }

    // 清理
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 0;
```

## 8.sqlite3_busy_timeout()

`sqlite3_busy_timeout()` 是 SQLite 中的一个函数，用于设置数据库连接的忙等待超时时间。当一个数据库连接尝试访问一个被其他进程或线程锁定的数据库文件时，SQLite 会等待一段时间，直到锁定被释放。如果等待时间超过设置的超时时间，SQLite 会返回一个错误。

### 1.函数介绍

```c
int sqlite3_busy_timeout(sqlite3 *db, int ms);
//参数说明
	-db：指向 sqlite3 类型的指针，表示数据库连接。
	-ms：忙等待超时时间，单位为毫秒。如果设置为 0，则禁用忙等待超时机制。
//返回值
	-SQLITE_OK：成功设置忙等待超时时间。
	-SQLITE_ERROR：设置失败。
	-其他错误代码：表示其他类型的错误。
```

### 2.使用场景

- **多线程或多进程环境**：在多线程或多进程环境中，多个线程或进程可能会同时访问同一个数据库文件。设置忙等待超时时间可以避免因数据库锁定而导致的无限等待。
- **防止死锁**：通过设置合理的超时时间，可以避免因数据库锁定而导致的死锁问题。

# SQL 注入攻击

## 1.注入攻击概念

SQL 注入攻击是一种常见的网络安全威胁，攻击者通过在输入字段中插入恶意的 SQL 代码，试图操纵数据库查询，从而获取未经授权的数据或执行非法操作。这种攻击通常利用应用程序在构建 SQL 查询时未能正确处理用户输入的漏洞。

## 2.SQL 注入攻击的工作原理
当应用程序将用户输入直接拼接到 SQL 查询中时，攻击者可以插入特殊的 SQL 代码，改变查询的结构和意图。例如，考虑一个简单的登录表单，用户输入用户名和密码，应用程序使用这些输入构建 SQL 查询：

```sql
SELECT * FROM users WHERE username = '输入的用户名' AND password = '输入的密码';
```

如果用户输入的用户名为 `admin' --`，密码为空，查询将变为：

```sql
SELECT * FROM users WHERE username = 'admin' -- AND password = '';
```

由于 `--` 是 SQL 中的注释符号，查询的实际效果是：

```sql
SELECT * FROM users WHERE username = 'admin';
```

这将绕过密码检查，允许攻击者以管理员身份登录。

## 3.防止 SQL 注入的方法
为了防止 SQL 注入攻击，可以采取以下几种方法：

### 1. 使用参数化查询
参数化查询是防止 SQL 注入的最有效方法之一。通过使用参数化查询，应用程序将用户输入作为参数传递给预编译的 SQL 语句，而不是直接拼接到查询中。这样可以确保用户输入被正确处理，不会改变查询的结构。

##### 示例代码（使用 SQLite）
```c
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    const char *sql;
    int rc;

    // 打开数据库
    rc = sqlite3_open("example.db", &db);
    if (rc) {
        fprintf(stderr, "无法打开数据库: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 创建表
    sql = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, username TEXT, password TEXT)";
    rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 插入数据
    sql = "INSERT INTO users (username, password) VALUES (?, ?)";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "无法准备 SQL 语句: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, "admin", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, "password123", -1, SQLITE_STATIC);

    // 执行插入操作
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "插入失败: %s\n", sqlite3_errmsg(db));
    }

    // 清理
    sqlite3_finalize(stmt);

    // 查询数据
    sql = "SELECT * FROM users WHERE username = ? AND password = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "无法准备 SQL 语句: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, "admin", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, "password123", -1, SQLITE_STATIC);

    // 执行查询并处理结果
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("ID: %d, Username: %s, Password: %s\n",
               sqlite3_column_int(stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_text(stmt, 2));
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "查询失败: %s\n", sqlite3_errmsg(db));
    }

    // 清理
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 0;
}
```

### 2. 使用存储过程
存储过程是在数据库中预定义的 SQL 代码块，可以接受参数并执行复杂的操作。使用存储过程可以减少直接在应用程序中拼接 SQL 查询的需求，从而降低 SQL 注入的风险。

### 3. 输入验证
对用户输入进行严格的验证，确保输入符合预期的格式。例如，如果某个字段只接受数字，可以使用正则表达式或其他验证方法确保输入只包含数字。

### 4. 最小权限原则
确保数据库用户只拥有完成其任务所需的最小权限。例如，如果一个应用程序只需要读取数据，不要授予其写入或删除数据的权限。

### 5. 使用 ORM 框架
对象关系映射（ORM）框架可以自动处理 SQL 查询的构建和执行，通常会自动使用参数化查询，从而减少 SQL 注入的风险。

### 6. 示例：输入验证

以下是一个简单的输入验证示例，确保用户名和密码只包含字母和数字：

```c
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int is_valid_input(const char *input) {
    while (*input) {
        if (!isalnum(*input)) {
            return 0; // 输入包含非字母数字字符
        }
        input++;
    }
    return 1; // 输入有效
}

int main() {
    char username[100];
    char password[100];

    printf("Enter username: ");
    scanf("%99s", username);

    printf("Enter password: ");
    scanf("%99s", password);

    if (!is_valid_input(username) || !is_valid_input(password)) {
        printf("Invalid input. Only alphanumeric characters are allowed.\n");
        return 1;
    }

    printf("Username: %s, Password: %s\n", username, password);

    return 0;
}
```

### 7.总结
SQL 注入攻击是一种严重的安全威胁，但通过使用参数化查询、存储过程、输入验证、最小权限原则和 ORM 框架等方法，可以有效防止这种攻击。在开发过程中，始终遵循安全最佳实践，确保应用程序的安全性。