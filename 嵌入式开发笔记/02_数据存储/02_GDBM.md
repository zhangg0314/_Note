# GDBM介绍

GDBM（GNU database manager）是一套简单的资料管理程序。一般的linux发行版中都会自带了一个符合X/Open技术规范的DBM数据库，这个数据库适合存储相对比较静态的索引化数据。有些人认为DBM根本算不上是个数据库，顶多算是个索引化的文件存储系统，事实也确实如此，因此操作GDBM就跟操作文件很相似。

GDBM是由GNU实现的DBM数据库，其特点是简单、小巧、可靠、高性能,并且已经被移植到了windows平台。GDBM数据库包含若干条记录，每一个记录由关键字和数据记录（KEY/VALUE）构成。保存在GDBM数据库中的每一个记录都必须有一个独一无二的关键字，数据可以是简单的数据类型，也可以是复杂的数据类型，例如C语言中的结构。技术规范里允许具体实现时把关键字和数据的最大长度限制在1024个字节，但这个限制通常没有什么意义，因为具体实现出来的东西往往比规范更灵活。

# GDBM特点

- 按key-value存储数据，value是可变长的。它只对key进行索引，只能按key进行查询。
- 高效的查询，低效的插入，适合于存储比较静态的数据。
- GDBM可以很容易地编译进一个可发布的二进制文件中，不需要独立地安装数据库服务器。
- 不支持SQL，不支持表之间创建关系，更不用说存储过程、触发器什么的了。

# GDBM数据类型

```c
typedef struct {
    int dummy[10];
} *GDBM_FILE;

/* The data and key structure. This structure is defined for compatibility. */
typedef struct {
	char *dptr;
	int dsize;
} datum; 
```

GDBM_FILE类型指向打开的数据库文件。datum是用typedef语句定义的类型，在使用GDBM数据库时，如果想引用一个数据记录，必须先声明一个datum类型的结构，然后让参数dptr指向数据库里数据记录的起始点，把数据记录的长度放在dsize参数里。

# GDBM接口函数

## 1.数据库的创建与打开

### 1.函数介绍

创建和打开数据库使用gdbm_open()函数,它返回一个GDBM_FILE类型的指针，指向打开的数据库文件。如果打开失败，则返回NULL。

```c
GDBM_FILE gdbm_open(char *name,int block_size,int read_write,int mode,void (*fatal_func) ()) 
    
//params:
	//- name:文件名，如果只是文件名，那么文件与程序在同一个目录内。
	//- block_size:指一次从硬盘写到内存的块的大小，最小为512。通常设置为0，这样GDBM将使用文件系统的默认值。
	//- read_write:
				GDBM_READER //读模式
				GDBM_WRITER //写模式
    			GDBM_WRCREAT //写模式，如果数据库文件不存在则创建数据库文件
				GDBM_NEWDB //写模式，如果数据库文件存在则覆盖原数据库文件
				//通过"|"操作符还可以改变GDBM数据库的写入操作方式，有下面两种可选:
						1.GDBM_FAST 在写入数据时，数据内容不会立刻保存到磁盘的数据库文件中，而是临时保存在内存里，若想将数据写入数据库文件,必须调用 gdbm_sync()函数. 该参数在gdbm-1.8之后成为默认值。
						2.GDBM_SYNC 写入的数据时，数据内容会立刻保存到磁盘的数据库文件中.
						3.还有一个特别的参数就是下面这个：GDBM_NOLOCK 通常情况下,在某进程已经打开某数据库文件时, 若有其他进程试图打开该数据库文件的话,就会引发Error,若指定了该标识的话, 就可以同时打开该数据库文件而不会引发Error了.

	//- mode:
				S_IRWXU 00700 允许文件的属主读、写和执行文件
				S_IRUSR(S_IREAD 00400 允许文件的属主读文件
				S_IWUSR(S_IWRITE) 00200 允许文件的属主写文件
				S_IXUSR(S_IEXEC) 00100 允许文件的属主执行文件
                        
				S_IRWXG 00070 允许文件所在的分组读、写和执行文件
				S_IRGRP 00040 允许文件所在的分组读文件
				S_IWGRP 00020 允许文件所在的分组写文件
				S_IXGRP 00010 允许文件所在的分组执行文件

				S_IRWXO 00007 允许其它用户读、写和执行文件
				S_IROTH 00004 允许其它用户读文件
				S_IWOTH 00002 允许其它用户写文件
				S_IXOTH 00001 允许其它用户执行文件

	//- fatal_func 
                 当出错时执行其指向的函数，通常设置为NULL,那样GDBM可以使用自己默认的函数。
```

### 2.示例

```c
GDBM_FILE dbf;
dbf = gdbm_open("mydata.db", 0, GDBM_READER, 0, NULL);
if( dbf == NULL ) {
  printf("Can not open database\n, %s\n", gdbm_strerror(gdbm_errno) );
} 
```

## 2.数据库的关闭

### 1.函数介绍

```c
//关闭已经打开的数据库
void gdbm_close (GDBM_FILE dbf)
//参数简介：
	dbf:指向数据库文件的指针
```

### 2.示例

```c
gdbm_close(dbf); 
```

## 3.数据的存储

### 1.函数介绍

GDBM数据库的存储方式是一个关键字对应一条数据记录，关键字类似于数据库中的主键，然而关键字并不一定为数据记录中的字段。GDBM数据库中定义了datum类型，无论是关键字还是数据记录都是以datum类型存储在数据库中的。GDBM不会创建两个一样的关键字；在gdbm_store（）函数中选用GDBM_INSERT参数，如果新数据记录的关键字已经存在，那么会返回1，但新的数据记录不会被插入到数据库中.gdbm_store（）函数成功插入数据会返回0，失败会返回-1.

```c
int gdbm_store (GDBM_FILE dbf,datum key,datum content,int flag) 
//params:
	dbf 指向数据库文件的指针
	key 关键字
	content: 记录的数据
	flag 
        GDBM_INSERT 插入新记录，如果记录已经存在则报错
	    GDBM_REPLACE 插入新记录，如果记录存在则覆盖
```

### 2.示例

```c
char keybuf[256], databuf[256];
datum key, data;

gets(keybuf);
gets(databuf);

key.dptr = keybuf; 
key.dsize = strlen(keybuf);

data.dptr = databuf; 
data.dsize = strlen(databuf);

rc = gdbm_store(dbf, key, data, GDBM_INSERT);
if( rc != 0 ) {
  printf("Can not store record\n, %s\n", gdbm_strerror(gdbm_errno) );
} 
```

## 4.数据的读取

### 1.函数介绍

根据关键字获取数据库中对应的数据记录.如果在数据库中没有找到关键字，那么返回的datum结构中的dptr将被置空(NULL).如果找到了关键字，返回的datum结构中的dptr会指向一段由malloc申请的内存，在使用完这些数据后需要释放。

```c
datum gdbm_fetch (GDBM_FILE dbf,datum key) 
//参数简介：
	dbf:指向数据库文件的指针
	key:关键字
```

### 2.示例

```c
char keybuf[256];
datum key, data;
gets(keybuf);

key.dptr = keybuf; 
key.dsize = strlen(keybuf);

data = gdbm_fetch(dbf, key);
if( data.dptr == NULL ) {
  puts("Record not found!\n");
}
else {
	printf("Record found (%d): %s", data.dsize, data.dptr);
  	free(data.dptr);
} 
```

## 5.数据的删除

### 1.函数介绍

删除一个关键字，注意只是删除了关键字，与该关键字关联的数据记录并没有被删除，如果想删除那个关键字关联的数据记录需要在调用gdbm_delete()之后调用gdbm_reorganize（）.如果关键字不存在或者文件是使用GDBM_READER参数打开的，那么这个函数会返回-1,成功返回0.

```c
int gdbm_delete (GDBM_FILE dbf,datum key)
//参数简介：
	dbf:指向数据库文件的指针
	key:关键字
```

### 2.示例

```c
char keybuf[256];
datum key;
gets(keybuf);

key.dptr = keybuf; 
key.dsize = strlen(keybuf);

rc = gdbm_delete(dbf, key);
if( rc != 0 )
	printf("Record can not delete.\n %s", gdbm_strerror(gdbm_errno));
```

## 6.数据的遍历

### 1.函数介绍

使用这两个函数可以依次遍历数据库中的所有数据记录。如果使用gdbm_open()函数的GDBM_WRITER(或者类似的)打开数据库,那么就需要小心操作，确定在遍历所有数据记录的过程中没有改变任何记录，否则会出现不能完全遍历所有数据记录的情况。如果返回的datum结构中的dptr为NULL,则表示已经遍历的所有的数据记录，已经到达数据库末尾了。

```c
datum gdbm_firstkey (GDBM_FILE dbf)
datum gdbm_nextkey (GDBM_FILE dbf,datum key)

//参数简介
	dbf:指向数据库文件的指针	
	key:关键字
```

### 2.示例

```c
datum key, nextkey;
int n = 1;
puts("Key list:\n");

key = gdbm_firstkey(dbf);

while( key.dptr != NULL ) {
	printf("%d: %s", n, key.dptr);
	n++;
	nextkey = gdbm_nextkey(dbf, key);
	free(key.dptr);
	key = nextkey;
}
```

## 7.其它GDBM函数

```c
int gdbm_reorganize (GDBM_FILE dbf)
    //在GDBM中, 因为删除所腾出空间将会被留作下次存储,所以删除元素后DB文件体积不会减小. 调用此函数后, 将重新生成数据库文件,这样可以避免浪费存储空间,删除大量数据后, 可以使用该方法来节省磁盘空间。
 
    
void gdbm_sync (GDBM_FILE dbf)
    //将元素的变更反映到文件中. 只有在FAST模式时才有效


int gdbm_exists (GDBM_FILE dbf,datum key)
    //检查数据库中是否有指定的关键字，一般会在读取数据库中的数据记录之前使用该函数。关键字存在，返回1，否则返回0.
    

char *gdbm_strerror (gdbm_error errno)
    //根据给定的错误代码返回一段描述错误的字符串信息

    
int gdbm_setopt (GDBM_FILE dbf,int option,int *value,int size)
	//可以使用gdbm_setopt()函数设置GDBM的行为
	//目前只有两个选项。
		- GDBM_CACHESIZE //设定GDBM数据库文件的缓存大小，默认值为100.
		- GDBM_FASTMODE //该选项允许你GDBM数据库切换到fast mode. 如果fast mode被开启(value置TRUE), GDBM将不会立刻将数据保存到磁盘的数据库文件中， 直到你调用gdbm_sync()函数才会将数据完全 保存到数据库文件里。
```

# GDBM错误处理

下面是错误代码，指出了出错类型，通过`gdbm_strerr（）`函数可以得到详细的出错信息。

| 错误宏                      | 解释                    |
| --------------------------- | ----------------------- |
| GDBM_NO_ERROR               | No error                |
| GDBM_MALLOC_ERROR           | Malloc error            |
| GDBM_BLOCK_SIZE_ERROR       | Block size error        |
| GDBM_FILE_OPEN_ERROR        | File open error         |
| GDBM_FILE_WRITE_ERROR       | File write error        |
| GDBM_FILE_SEEK_ERROR        | File seek error         |
| GDBM_FILE_READ_ERROR        | File read error         |
| GDBM_BAD_MAGIC_NUMBER       | Bad magic number        |
| GDBM_EMPTY_DATABASE         | Empty database          |
| GDBM_CANT_BE_READER         | Can't be reader         |
| GDBM_CANT_BE_WRITER         | Can't be writer         |
| GDBM_READER_CANT_DELETE     | Reader can't delete     |
| GDBM_READER_CANT_STORE      | Reader can't store      |
| GDBM_READER_CANT_REORGANIZE | Reader can't reorganize |
| GDBM_UNKNOWN_UPDATE         | Unknown update          |
| GDBM_ITEM_NOT_FOUND         | Item not found          |
| GDBM_REORGANIZE_FAILED      | Reorganize failed       |
| GDBM_CANNOT_REPLACE         | Cannot replace          |
| GDBM_ILLEGAL_DATA           | Illegal data            |
| GDBM_OPT_ALREADY_SET        | Option already set      |
| GDBM_OPT_ILLEGAL            | Illegal option          |
