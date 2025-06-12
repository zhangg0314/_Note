# XML简介

XML本质就是一个文件，文件在任何系统都是兼容可用的。XML 被设计用来传输和存储数据，而HTML 被设计用来显示数据。XML 指可扩展标记语言（eXtensible Markup Language）。可扩展标记语言（英语：Extensible Markup Language，简称：XML）是一种标记语言，是从标准通用标记语言（SGML）中简化修改出来的。它主要用到的有可扩展标记语言、可扩展样式语言（XSL）、XBRL和XPath等。

- XML 指可扩展标记语言（EXtensible Markup Language）。
- XML 是一种很像HTML的标记语言。
- XML 的设计宗旨是传输数据，而不是显示数据。
- XML 标签没有被预定义。您需要自行定义标签。
- XML 被设计为具有自我描述性。
- XML 是 W3C 的推荐标准。

------

# XML用途

## 1.不会做任何事

XML 不会做任何事情。XML 被设计用来结构化、存储以及传输信息。下面实例是 Jani 写给 Tove 的便签，存储为 XML：

```xml
<note> 
    <to>Tove</to> 
    <from>Jani</from> 
    <heading>Reminder</heading> 
    <body>Don't forget me this weekend!</body> 
</note>
```

上面的这条便签具有自我描述性。它包含了发送者和接受者的信息，同时拥有标题以及消息主体。但是，这个 XML 文档仍然没有做任何事情。它仅仅是包装在 XML 标签中的纯粹的信息。我们需要编写软件或者程序，才能传送、接收和显示出这个文档。

## 2.把数据从 HTML 分离

如果需要在 HTML 文档中显示动态数据，那么每当数据改变时将花费大量的时间来编辑 HTML。通过 XML，数据能够存储在独立的 XML 文件中。这样就可以专注于使用 HTML/CSS 进行显示和布局，并确保修改底层数据不再需要对 HTML 进行任何的改变。通过使用几行 JavaScript 代码，就可以读取一个外部 XML 文件，并更新网页的数据内容。

## 3.简化数据共享

在真实的世界中，计算机系统和数据使用不兼容的格式来存储数据。XML 数据以纯文本格式进行存储，因此提供了一种独立于软件和硬件的数据存储方法。这让创建不同应用程序可以共享的数据变得更加容易。

## 4.简化数据传输

对开发人员来说，其中一项最费时的挑战一直是在互联网上的不兼容系统之间交换数据。由于可以通过各种不兼容的应用程序来读取数据，以 XML 交换数据降低了这种复杂性。

## 5.简化平台变更

升级到新的系统（硬件或软件平台），总是非常费时的。必须转换大量的数据，不兼容的数据经常会丢失。XML 数据以文本格式存储。这使得 XML 在不损失数据的情况下，更容易扩展或升级到新的操作系统、新的应用程序或新的浏览器。

## 6.使数据更有用

不同的应用程序都能够访问数据，不仅仅在 HTML 页中，也可以从 XML 数据源中进行访问。通过 XML，数据可供各种阅读设备使用（掌上计算机、语音设备、新闻阅读器等），还可以供盲人或其他残障人士使用。

## 7.用于创建新的互联网语言

很多新的互联网语言是通过 XML 创建的。这里有一些实例：

- XHTML
- 用于描述可用的 Web 服务 的 WSDL
- 作为手持设备的标记语言的 WAP 和 WML
- 用于新闻 feed 的 RSS 语言
- 描述资本和本体的 RDF 和 OWL
- 用于描述针对 Web 的多媒体 的 SMIL

# XML的格式

```xml
<?xml version="1.0" encoding="utf-8" ?> <!--这行是必须要写的，且必须放在首行（前面有注释都不行）。表示版本为1.0，以utf-8字符集来编码-->
 
<!--Users是一个根标签，必须只能有一个，而Users里面的子属性可以有多个,根标签只能有一个，子标签可以有多个-->
<!--根标签名字随意取,标签是成对存在的，记得嵌套正确-->
<Users>
    <user id="1"><!--id表示user节点的属性-->
        <name>张三</name>
        <age>18</age>
        <address>广州</address>
    </user>
    <userAttribute>都是爱学习的人</userAttribute>
    <user id="2">
        <name>李四</name>
        <age>25</age>
        <address>哈尔滨</address>
    </user>
 
    <!--以下是带有大于号小于号等特殊字符的写法-->
    <special>
        <![CDATA[ 5 > 2 && 3 < 5 ]]>
    </special>
    <!--特殊字符用法二-->
    <special>  5 &gt; 2 &amp;&amp; 3 &lt; 5 </special>
</Users>
```

![image-20250611154345101](..\figure\image-20250611154345101.png)

------

# ——XML文件解析——

# 1.tinyxml

tinyxml使用的是DOM解析方式，优点是解析速度快，缺点是无法解析复杂的xml文件。

## 1.创建

创建一个xml文件，并为其插入头部信息和根、子节点

### 1.创建一个XML类

```c++
TiXmlDocument* tinyXmlDoc = new TiXmlDocument();
```

### 2.创建头部信息并插入到xml类中

```c++
// xml的声明(三个属性：版本，编码格式，保留空串即可)
TiXmlDeclaration* tinyXmlDeclare = new TiXmlDeclaration("1.0", "utf-8", "");	// 声明头部格式
// 插入文档类中
tinyXmlDoc->LinkEndChild(tinyXmlDeclare);
```

### 3.创建根节点

```c++
// 创建时需要指定根节点的名称
TiXmlElement* Library = new TiXmlElement("Library");
tinyXmlDoc->LinkEndChild(Library);		// 把根节点插入到文档类中
```

![image-20250611155416596](..\figure\image-20250611155416596.png)

### 4.添加子节点

首先创建一个子节点，然后新建一个文本，将文本添加到子节点中，最后将字节点添加到根节点中。

```c++
TiXmlElement *Book = new TiXmlElement("Book");
TiXmlText *bookText = new TiXmlText("书本");	// 创建文本
Book->LinkEndChild(bookText);	// 给Book节点添加文本
Library->LinkEndChild(Book);	// 插入到根节点下
```

![image-20250611155527301](..\figure\image-20250611155527301.png)

### 5.添加带属性的子节点

使用setAttribute可以简单的给节点添加属性值；如果要创建子节点的子节点，就跟创建子节点一样，只是创建好后要添加到子节点中即可。

```c++
TiXmlElement *Book1 = new TiXmlElement("Book1");
// 插入属性
Book1->SetAttribute("ID", 1);
Book1->SetAttribute("Name", "水浒传");
Book1->SetAttribute("Price", "64.6");

// 创建Book1的子节点Description
TiXmlElement *Description = new TiXmlElement("Description");
TiXmlText *descriptionText = new TiXmlText("108个拆迁户");		// 创建文本
Description->LinkEndChild(descriptionText);		// 给Description节点添加文本
Book1->LinkEndChild(Description);				// 插入到Book1节点下

// 创建Book1的子节点Page
TiXmlElement *Page = new TiXmlElement("Page");
TiXmlText *pageText = new TiXmlText("100页");		// 创建文本
Page->LinkEndChild(pageText);	// 给Page节点添加文本
Book1->LinkEndChild(Page);		// 插入到Book1节点下

Library->LinkEndChild(Book1);	// 插入到根节点下
```

如图：![image-20250611155805362](..\figure\image-20250611155805362.png)

### 6.将doc写入xml文件

只需使用文档类调用SaveFile方法即可，参数传文件名,写入成功返回true，写入失败返回false

```c++
bool result = tinyXmlDoc->SaveFile(FILE_NAME);
```

### 7.转换为字符串

```c++
TiXmlPrinter printer;
tinyXmlDoc->Accept(&printer);
printf("%s\n", printer.CStr());
//如果只是想在控制台打印而已，那么可以直接调用函数tinyXmlDoc->Print();就可以实现在控制台上打印出来了！
```

### 8.创建xml文件并插入元素节点

```c++
void create_XML() {
	printf("\n----- create_XML -----\n"); 

	//新建一个xml文件
	// 定义一个TiXmlDocument类指针
	TiXmlDocument* tinyXmlDoc = new TiXmlDocument();

	// xml的声明(三个属性：版本，编码格式，保留空串即可)
	TiXmlDeclaration* tinyXmlDeclare = new TiXmlDeclaration("1.0", "utf-8", "");	// 声明头部格式
	// 插入文档类中
	tinyXmlDoc->LinkEndChild(tinyXmlDeclare);

	// 根节点
	TiXmlElement* Library = new TiXmlElement("Library");
	tinyXmlDoc->LinkEndChild(Library);// 把根节点插入到文档类中

	// 创建Book节点
	TiXmlElement *Book = new TiXmlElement("Book");
	TiXmlText *bookText = new TiXmlText("书本");	// 创建文本
	Book->LinkEndChild(bookText);	// 给Book节点添加文本
	Library->LinkEndChild(Book);	// 插入到根节点下

	// 创建Book1节点
	TiXmlElement *Book1 = new TiXmlElement("Book1");
	// 插入属性
	Book1->SetAttribute("ID", 1);
	Book1->SetAttribute("Name", "水浒传");
	Book1->SetAttribute("Price", "64.6");

	// 创建Book1的子节点Description
	TiXmlElement *Description = new TiXmlElement("Description");
	TiXmlText *descriptionText = new TiXmlText("108个拆迁户");		// 创建文本
	Description->LinkEndChild(descriptionText);		// 给Description节点添加文本
	Book1->LinkEndChild(Description);				// 插入到Book1节点下

	// 创建Book1的子节点Page
	TiXmlElement *Page = new TiXmlElement("Page");
	TiXmlText *pageText = new TiXmlText("100页");		// 创建文本
	Page->LinkEndChild(pageText);	// 给Page节点添加文本
	Book1->LinkEndChild(Page);		// 插入到Book1节点下

	Library->LinkEndChild(Book1);	// 插入到根节点下

	// 保存到文件	
	bool result = tinyXmlDoc->SaveFile(FILE_NAME);
	if (result == true) printf("文件写入成功！\n");
	else printf("文件写入失败！\n");

	// 打印出来看看
	//tinyXmlDoc->Print();
    
	TiXmlPrinter printer;
	tinyXmlDoc->Accept(&printer);
	printf("%s\n", printer.CStr());
}
```



例：在上面创建的xml文件基础上，添加两个Book1子节点。

1. 定义一个xml文件类，并读取文件中的xml内容初始化它
这里有两种方式，有点奇怪，当使用第一种方式读取xml文件时，xml中有多个子节点Book1时，由于属性都有相同的id、name、price，导致会读取失败，不得已让我搞出了第二种方式去读取进行初始化，这样就没问题！

1). 方式1
// 定义一个TiXmlDocument类指针
TiXmlDocument* tinyXmlDoc = new TiXmlDocument;

// 读取文件中的xml
if (!tinyXmlDoc->LoadFile(FILE_NAME)) {
	// 读取失败，打印失败原因
	printf("Could not load example xml file %s. Error='%s'\n", FILE_NAME, tinyXmlDoc->ErrorDesc());
	return ;
}
1
2
3
4
5
6
7
8
9
读取失败显示：


2). 方式2
定义xml文档类对象时指定文件名，然后在使用这个对象去调用LoadFile传参TIXML_ENCODING_LEGACY即可，具体传参是什么意思，我也不清楚，自己摸索出来的，其中有三个枚举可以选择，其他两个都不行，就这个可以！

// 定义xml文档类对象时指定文件名
TiXmlDocument* tinyXmlDoc = new TiXmlDocument(FILE_NAME);
// 然后在使用这个对象去调用LoadFile传参TIXML_ENCODING_LEGACY即可
tinyXmlDoc->LoadFile(TIXML_ENCODING_LEGACY);
1
2
3
4
我一般都是，第一种方式用不了，就用第二种方式初始化！

3). 对已存在的数据进行初始化
std::string data;	// 这里默认data是存储xml数据的字符串变量

/* 方式一： */
// 定义一个TiXmlDocument类指针
TiXmlDocument* tinyXmlDoc = new TiXmlDocument();
// 数据初始化
tinyXmlDoc->Parse(data.c_str());

/* 方式二： */
// 定义一个TiXmlDocument类指针
TiXmlDocument* tinyXmlDoc = new TiXmlDocument(data.c_str());
1
2
3
4
5
6
7
8
9
10
11
4). 也可以获取头部信息中的版本号和编码
// 读取文档声明信息(也就是xml的头部信息：<?xml version="1.0" encoding="utf-8" ?>)
TiXmlDeclaration *pDeclar = tinyXmlDoc->FirstChild()->ToDeclaration();
if (pDeclar != NULL) {
	printf("头部信息： version is %s , encoding is %s\n", pDeclar->Version(), pDeclar->Encoding());
}
1
2
3
4
5
2. 获取根节点
这个定义是需要指定参数根节点的名字，然后再通过RootElement方法获取。

TiXmlElement *Library = new TiXmlElement("Library");
Library = tinyXmlDoc->RootElement();
1
2
3. 创建Book1节点，并设置属性
TiXmlElement *Book1 = new TiXmlElement("Book1");
// 插入属性
Book1->SetAttribute("ID", 2);
Book1->SetAttribute("Name", "西游记");
Book1->SetAttribute("Price", "99.81");
1
2
3
4
5
假设创建好后，在xml文件中是这样显示的：


4. 创建Book1的子节点Description 和 Page
和上面创建的思路代码是一样的，创建好后在插入到Book1节点中就好了

// 创建Book1的子节点Description
TiXmlElement *Description = new TiXmlElement("Description");
TiXmlText *descriptionText = new TiXmlText("师徒四人");		// 创建文本
Description->LinkEndChild(descriptionText);		// 给Description节点添加文本
Book1->LinkEndChild(Description);				// 插入到Book1节点下

// 创建Book1的子节点Page
TiXmlElement *Page = new TiXmlElement("Page");
TiXmlText *pageText = new TiXmlText("81页");		// 创建文本
Page->LinkEndChild(pageText);	// 给Page节点添加文本
Book1->LinkEndChild(Page);		// 插入到Book1节点下
1
2
3
4
5
6
7
8
9
10
11
假设创建好后，在xml文件中是这样显示的：


5. 将Book1节点添加到根节点中
Library->LinkEndChild(Book1);	// 插入到根节点下
1
6. 将doc写入xml文件
// 保存到文件	
bool result = tinyXmlDoc->SaveFile(FILE_NAME);
if (result == true) printf("文件写入成功！\n");
else printf("文件写入失败！\n");
1
2
3
4
7. 为已存在的xml文件添加节点元素，总代码如下
	void add_XML() {
	printf("\n----- add_XML -----\n");

	// 定义一个TiXmlDocument类指针
	TiXmlDocument* tinyXmlDoc = new TiXmlDocument(FILE_NAME);
	tinyXmlDoc->LoadFile(TIXML_ENCODING_LEGACY);

	// 读取文档声明信息(也就是xml的头部信息：<?xml version="1.0" encoding="utf-8" ?>)
	TiXmlDeclaration *pDeclar = tinyXmlDoc->FirstChild()->ToDeclaration();
	if (pDeclar != NULL) {
		printf("头部信息： version is %s , encoding is %s\n", pDeclar->Version(), pDeclar->Encoding());
	}

	// 得到文件根节点
	TiXmlElement *Library = new TiXmlElement("Library");
	Library = tinyXmlDoc->RootElement();


	// 创建Book1节点
	TiXmlElement *Book1 = new TiXmlElement("Book1");
	// 插入属性
	Book1->SetAttribute("ID", 2);
	Book1->SetAttribute("Name", "西游记");
	Book1->SetAttribute("Price", "99.81");
	
	// 创建Book1的子节点Description
	TiXmlElement *Description = new TiXmlElement("Description");
	TiXmlText *descriptionText = new TiXmlText("师徒四人");		// 创建文本
	Description->LinkEndChild(descriptionText);		// 给Description节点添加文本
	Book1->LinkEndChild(Description);				// 插入到Book1节点下
	
	// 创建Book1的子节点Page
	TiXmlElement *Page = new TiXmlElement("Page");
	TiXmlText *pageText = new TiXmlText("81页");		// 创建文本
	Page->LinkEndChild(pageText);	// 给Page节点添加文本
	Book1->LinkEndChild(Page);		// 插入到Book1节点下
	
	Library->LinkEndChild(Book1);	// 插入到根节点下




	// 创建Book1节点
	Book1 = new TiXmlElement("Book1");
	// 插入属性
	Book1->SetAttribute("ID", 3);
	Book1->SetAttribute("Name", "三国演义");
	Book1->SetAttribute("Price", "66.66");
	
	// 创建Book1的子节点Description
	Description = new TiXmlElement("Description");
	descriptionText = new TiXmlText("三国大战");		// 创建文本
	Description->LinkEndChild(descriptionText);		// 给Description节点添加文本
	Book1->LinkEndChild(Description);				// 插入到Book1节点下
	
	// 创建Book1的子节点Page
	Page = new TiXmlElement("Page");
	pageText = new TiXmlText("30页");		// 创建文本
	Page->LinkEndChild(pageText);	// 给Page节点添加文本
	Book1->LinkEndChild(Page);		// 插入到Book1节点下
	
	Library->LinkEndChild(Book1);	// 插入到根节点下


	// 保存到文件	
	bool result = tinyXmlDoc->SaveFile(FILE_NAME);
	if (result == true) printf("文件写入成功！\n");
	else printf("文件写入失败！\n");
	
	// 打印出来看看
	tinyXmlDoc->Print();
}

1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
19
20
21
22
23
24
25
26
27
28
29
30
31
32
33
34
35
36
37
38
39
40
41
42
43
44
45
46
47
48
49
50
51
52
53
54
55
56
57
58
59
60
61
62
63
64
65
66
67
68
69
70
71
72
执行完如上代码，会在xml文件中添加指定的节点元素，如下图

可以看到，创建相同子节点的代码都是一样的，也就是说，当需要创建几十个这个的子节点时，可以使用for循环去处理，至于数据可以使用数组是事先存储即可！

四、删除
删除子节点

例：
/* 删除book 和 其中一个book1节点 和 其中一个属性 */

1. 定义一个xml文件类，并读取文件中的xml内容初始化它
和上面 三、添加 的第1步骤 一样…

2. 获取根节点
这个定义是需要指定参数根节点的名字，然后再通过RootElement方法获取。

// 创建时需要指定名称
TiXmlElement *Library = new TiXmlElement("Library");
Library = tinyXmlDoc->RootElement();
1
2
3
3. 删除Book节点
像这种只有自己本身的，没有其他子节点的节点，它有两种方式去删除

1). 方式一
获取到对应节点后调用Clear()方法去删除。

// 方式一
TiXmlElement* Book = Library->FirstChildElement("Book");
Book->Clear();
1
2
3
2). 方式二
获取到对应节点后，根节点调用RemoveChild去删除，传参时需要对变量转换一下

// 获取Book节点
TiXmlElement* Book = Library->FirstChildElement("Book");
Library->RemoveChild(Book->ToElement());
1
2
3
建议还是使用第二种方式

a. 当然，如果Book节点不止一个时，如果需要删除特定的几个，可以使用for循环去删除
判断文本，符合条件就删除

调用FirstChildElement方法，传入子节点名字，可以获得第一个相同名字的子节点
调用NextSiblingElement方法，传入节点名字，可以获得下一个相同名字的节点

调用RemoveChild方法，是父节点删除子节点的方法

TiXmlElement* pItem1 = Library->FirstChildElement("Book");
for (; pItem1 != NULL; ) {

	if (strncmp(pItem1->GetText(), "书本", 6) == 0) {
		// 提前存储删除节点的下一个节点
		TiXmlElement* temporary = pItem1->NextSiblingElement("Book1");
	
		// 删除当前节点，删除后pItem为NULL，如果再继续使用它会报错
		Library->RemoveChild(pItem1->ToElement());
	
		// 所以这里要进行赋值回来
		pItem1 = temporary;
		
	} else {
		// 寻找下一个Book1节点
		pItem1 = pItem1->NextSiblingElement("Book1");
	}
}

1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
用if判断，然后符合条件进行删除即可!

这里讲一下if里面的操作：
首先删除当前pItem1后，pItem1就变为NULL值了，如果在进行判断pItem1，它就是等于NULL的，那么就直接结束循环了，有时候不等于NULL，再去使用它去获取下一个节点会报错。
所以，在删除之前，先获取下一个节点保存着，然后进行删除，最后将赋值回来就可以了！

这个也是自己摸索出来的，不知道会不会有什么隐藏的BUG，反正目前我测试着是没有问题的！
也许还会有其他更好的方法，目前我是想不出来了，如果有想出来的朋友，可以评论区发出来，互相学习学习，谢谢！

假设执行完，xml中将会是这样：


4. 删除属性
可以使用直接写全代码的方式去删除，如上面的方式二一样，也可以使用for循环，然后用符合条件后去删除，这里就是使用了for循环去删除属性。

删除属性直接删除即可，不需要做其他什么操作！

调用Attribute方法，传入属性名可以获得属性的值
调用RemoveAttribute方法，传入属性名可以进行删除

// 删除属性
TiXmlElement* pItem = Library->FirstChildElement("Book1");
for (; pItem != NULL; pItem = pItem->NextSiblingElement("Book1")) {

	// 找到属性ID = 1的节点
	if (strcmp(pItem->Attribute("ID"), "1") == 0) {
		// 删除属性为Name的属性
		pItem->RemoveAttribute("Name");
	}
}
1
2
3
4
5
6
7
8
9
10
假设执行完，xml中将会是这样：


5. 删除整个节点
思路和上面删除Book节点一样

这里是删除ID属性为2的Book1节点

pItem = Library->FirstChildElement("Book1");
for (; pItem != NULL; ) {

	// 找到属性ID = 2的节点
	if (strcmp(pItem->Attribute("ID"), "2") == 0) {
		// 提前存储删除节点的下一个节点
		TiXmlElement* temporary = pItem->NextSiblingElement("Book1");
	
		// 删除当前节点，删除后pItem为NULL，如果再继续使用它会报错
		Library->RemoveChild(pItem->ToElement());
	
		// 所以这里要进行赋值回来
		pItem = temporary;
		
	} else {
		// 寻找下一个Book1节点
		pItem = pItem->NextSiblingElement("Book1");
	}
}

1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
19
假设执行完，xml中将会是这样：


6. 将doc写入xml文件
// 保存到文件	
bool result = tinyXmlDoc->SaveFile(FILE_NAME);
if (result == true) printf("文件写入成功！\n");
else printf("文件写入失败！\n");
1
2
3
4
7. 删除总代码如下
	void del_XML() {
	printf("\n----- del_XML -----\n");

	// 定义一个TiXmlDocument类指针
	TiXmlDocument* tinyXmlDoc = new TiXmlDocument(FILE_NAME);
	tinyXmlDoc->LoadFile(TIXML_ENCODING_LEGACY);

	// 读取文档声明信息(也就是xml的头部信息：<?xml version="1.0" encoding="utf-8" ?>)
	TiXmlDeclaration *pDeclar = tinyXmlDoc->FirstChild()->ToDeclaration();
	if (pDeclar != NULL) {
		printf("头部信息： version is %s , encoding is %s\n", pDeclar->Version(), pDeclar->Encoding());
	}

	// 得到文件根节点
	TiXmlElement *Library = new TiXmlElement("Library");
	Library = tinyXmlDoc->RootElement();


	/* 删除 Book 节点 */
	// 方式一
	//TiXmlElement* Book = Library->FirstChildElement("Book");
	//Book->Clear();


	// 方式二
	// 获取Book节点
	TiXmlElement* Book = Library->FirstChildElement("Book");
	Library->RemoveChild(Book->ToElement());


	//TiXmlElement* pItem1 = Library->FirstChildElement("Book");
	//for (; pItem1 != NULL; ) {
	
	//	if (strncmp(pItem1->GetText(), "书本", 6) == 0) {
	//		// 提前存储删除节点的下一个节点
	//		TiXmlElement* temporary = pItem1->NextSiblingElement("Book1");
	
	//		// 删除当前节点，删除后pItem为NULL，如果再继续使用它会报错
	//		Library->RemoveChild(pItem1->ToElement());
	
	//		// 所以这里要进行赋值回来
	//		pItem1 = temporary;
	//	
	//	} else {
	//		// 寻找下一个Book1节点
	//		pItem1 = pItem1->NextSiblingElement("Book1");
	//	}
	//}



	// 删除属性
	TiXmlElement* pItem = Library->FirstChildElement("Book1");
	for (; pItem != NULL; pItem = pItem->NextSiblingElement("Book1")) {
	
		// 找到属性ID = 1的节点
		if (strcmp(pItem->Attribute("ID"), "1") == 0) {
			// 删除属性为Name的属性
			pItem->RemoveAttribute("Name");
		}
	}
	
	// 删除整个节点
	pItem = Library->FirstChildElement("Book1");
	for (; pItem != NULL; ) {
	
		// 找到属性ID = 2的节点
		if (strcmp(pItem->Attribute("ID"), "2") == 0) {
			// 提前存储删除节点的下一个节点
			TiXmlElement* temporary = pItem->NextSiblingElement("Book1");
	
			// 删除当前节点，删除后pItem为NULL，如果再继续使用它会报错
			Library->RemoveChild(pItem->ToElement());
	
			// 所以这里要进行赋值回来
			pItem = temporary;
		
		} else {
			// 寻找下一个Book1节点
			pItem = pItem->NextSiblingElement("Book1");
		}
	}




	// 保存到文件	
	bool result = tinyXmlDoc->SaveFile(FILE_NAME);
	if (result == true) printf("文件写入成功！\n");
	else printf("文件写入失败！\n");
	
	// 打印出来看看
	tinyXmlDoc->Print();

}

1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
19
20
21
22
23
24
25
26
27
28
29
30
31
32
33
34
35
36
37
38
39
40
41
42
43
44
45
46
47
48
49
50
51
52
53
54
55
56
57
58
59
60
61
62
63
64
65
66
67
68
69
70
71
72
73
74
75
76
77
78
79
80
81
82
83
84
85
86
87
88
89
90
91
92
93
94
95
执行后xml文件如下图：


五、修改
修改(更改)XML中节点的值

例：将ID属性为3的Book1节点的Price属性值修改为33.33；将ID属性为1的Book1节点的Description子节点值修改为108个没房住，Page修改为999页。

1. 定义一个xml文件类，并读取文件中的xml内容初始化它
和上面 三、添加 的第1步骤 一样…

2. 获取根节点
和上面 三、添加 的第2步骤 一样…

3. 修改属性值
符合条件后使用SetAttribute方法即可进行重新修改属性

TiXmlElement* pItem = Library->FirstChildElement("Book1");
for (; pItem != NULL; pItem = pItem->NextSiblingElement("Book1")) {

	// 设置属性
	if (strcmp(pItem->Attribute("ID"), "3") == 0) {
		pItem->SetAttribute("Price", "33.33");
	}
}
1
2
3
4
5
6
7
8
执行后xml文件如下图：


4. 修改节点值
首先使用FirstChildElement获得对应节点后，在使用FirstChild获得需要修改的文本指针对象，最后在调用SetValue就可以进行修改了

TiXmlElement* pItem = Library->FirstChildElement("Book1");
for (; pItem != NULL; pItem = pItem->NextSiblingElement("Book1")) {

	// 找到属性ID = 1的节点
	if (strcmp(pItem->Attribute("ID"), "1") == 0) {
		// 设置Book1的子节点Description的值
		TiXmlElement* Description = pItem->FirstChildElement("Description");	// 获得<Description>108个没房住</Description>
		TiXmlNode* des = Description->FirstChild();	// 获取元素指针	// 获得存储 108个没房住 的指针
		des->SetValue("108个没房住");	// 重新为其设置值
	
		TiXmlElement *Page = pItem->FirstChildElement("Page");
		TiXmlNode *page = Page->FirstChild();
		page->SetValue("999页");
	}
}
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
执行后xml文件如下图：


其实修改属性和修改节点值可以放在同一个for循环中去操作的！

5. 将doc写入xml文件
// 保存到文件	
bool result = tinyXmlDoc->SaveFile(FILE_NAME);
if (result == true) printf("文件写入成功！\n");
else printf("文件写入失败！\n");
1
2
3
4
6. 修改xml中的节点值，代码汇总
	void alt_XML() {
	printf("\n----- alt_XML -----\n");

	// 定义一个TiXmlDocument类指针
	TiXmlDocument* tinyXmlDoc = new TiXmlDocument(FILE_NAME);
	tinyXmlDoc->LoadFile(TIXML_ENCODING_LEGACY);

	// 读取文档声明信息(也就是xml的头部信息：<?xml version="1.0" encoding="utf-8" ?>)
	TiXmlDeclaration *pDeclar = tinyXmlDoc->FirstChild()->ToDeclaration();
	if (pDeclar != NULL) {
		printf("头部信息： version is %s , encoding is %s\n", pDeclar->Version(), pDeclar->Encoding());
	}

	// 得到文件根节点
	TiXmlElement *Library = new TiXmlElement("Library");
	Library = tinyXmlDoc->RootElement();


	TiXmlElement* pItem = Library->FirstChildElement("Book1");
	for (; pItem != NULL; pItem = pItem->NextSiblingElement("Book1")) {
	
		// 找到属性ID = 1的节点
		if (strcmp(pItem->Attribute("ID"), "1") == 0) {
			// 设置Book1的子节点Description的值
			TiXmlElement* Description = pItem->FirstChildElement("Description");	// 获得<Description>108个没房住</Description>
			TiXmlNode* des = Description->FirstChild();	// 获取元素指针		// 获得存储 108个没房住 的指针
			des->SetValue("108个没房住");	// 重新为其设置值				
	
			TiXmlElement *Page = pItem->FirstChildElement("Page");
			TiXmlNode *page = Page->FirstChild();
			page->SetValue("999页");
		}
	
		// 设置属性
		if (strcmp(pItem->Attribute("ID"), "3") == 0) {
			pItem->SetAttribute("Price", "33.33");
		}
	}


	// 保存到文件	
	bool result = tinyXmlDoc->SaveFile(FILE_NAME);
	if (result == true) printf("文件写入成功！\n");
	else printf("文件写入失败！\n");
	//printf("%s\n", tinyXmlDoc->Value());
	// 打印出来看看
	tinyXmlDoc->Print();
}

1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
19
20
21
22
23
24
25
26
27
28
29
30
31
32
33
34
35
36
37
38
39
40
41
42
43
44
45
46
47
48
执行完如下效果：


六、解析
解析指定节点中值

1. 定义一个xml文件类，并读取文件中的xml内容初始化它
和上面 三、添加 的第1步骤 一样…

2. 获取根节点
和上面 三、添加 的第2步骤 一样…

3. 解析Book节点
当然，前面已将Book节点删掉了，所以获取到的Book为NULL

所以如果是指针，在使用前最好先判断一下再去使用！

TiXmlElement* Book = Library->FirstChildElement("Book");
if (Book) {
	printf("Book = %s\n\n", Book->GetText());
}
1
2
3
4
4. 解析所有的Book1节点的属性
直接调用Attribute，传入属性名即可获得属性的值；

/* 当有多个相同名字的节点时，可以使用循环进行读取解析 */
// 函数FirstChildElement()	:	找到指定名字的元素
// 函数NextSiblingElement	:	在同一级元素中查找下一个指定名字的元素
TiXmlElement* pItem = Library->FirstChildElement("Book1");
if (pItem) {
	for (; pItem != NULL; pItem = pItem->NextSiblingElement("Book1")) {

		// 解析属性
		printf("ID = %s\n", pItem->Attribute("ID"));
		printf("Name = %s\n", pItem->Attribute("Name"));
		printf("Price = %s\n", pItem->Attribute("Price"));
	
		printf("\n\n");
	}
}
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
5. 解析所有的Book1节点的子节点
获得对应解析的节点后，调用GetText可以获得里面的值；

/* 当有多个相同名字的节点时，可以使用循环进行读取解析 */
TiXmlElement* pItem = Library->FirstChildElement("Book1");
if (pItem) {
	for (; pItem != NULL; pItem = pItem->NextSiblingElement("Book1")) {
		// 解析Book1的子节点
		TiXmlElement* Description = pItem->FirstChildElement("Description");
		printf("Description = %s\n", Description->GetText());

		TiXmlElement *Page = pItem->FirstChildElement("Page");
		printf("Page = %s\n", Page->GetText());
	
		printf("\n\n");
	}
}
1
2
3
4
5
6
7
8
9
10
11
12
13
14
解析控制台输出如下


6. 解析xml节点值，代码汇总
	void parse_XML() {
	printf("\n----- parse_XML -----\n");

/* 方式一：当读取有问题时，可以使用下面方式二

	// 定义一个TiXmlDocument类指针
	TiXmlDocument* tinyXmlDoc = new TiXmlDocument;
	
	// 读取文件中的xml
	if (!tinyXmlDoc->LoadFile(FILE_NAME)) {
		// 读取失败，打印失败原因
		printf("Could not load example xml file %s. Error='%s'\n", FILE_NAME, tinyXmlDoc->ErrorDesc());
		return ;
	}
*/

	// 定义一个TiXmlDocument类指针
	TiXmlDocument* tinyXmlDoc = new TiXmlDocument(FILE_NAME);
	tinyXmlDoc->LoadFile(TIXML_ENCODING_LEGACY);
	
	// 读取文档声明信息(也就是xml的头部信息：<?xml version="1.0" encoding="utf-8" ?>)
	TiXmlDeclaration* pDeclar = tinyXmlDoc->FirstChild()->ToDeclaration();
	if (pDeclar != NULL) {
		printf("头部信息：version is %s , encoding is %s\n", pDeclar->Version(), pDeclar->Encoding());
	}
	
	// 得到文件根节点
	TiXmlElement* Library = new TiXmlElement("Library");
	if (Library) {
		Library = tinyXmlDoc->RootElement();
	}


	// 解析Book节点
	TiXmlElement* Book = Library->FirstChildElement("Book");
	if (Book) {
		printf("Book = %s\n\n", Book->GetText());
	}



	/* 当有多个相同名字的节点时，可以使用循环进行读取解析 */
	// 函数FirstChildElement()	:	找到指定名字的元素
	// 函数NextSiblingElement	:	在同一级元素中查找下一个指定名字的元素
	TiXmlElement* pItem = Library->FirstChildElement("Book1");
	if (pItem) {
		for (; pItem != NULL; pItem = pItem->NextSiblingElement("Book1")) {
	
			// 解析属性
			printf("ID = %s\n", pItem->Attribute("ID"));
			printf("Name = %s\n", pItem->Attribute("Name"));
			printf("Price = %s\n", pItem->Attribute("Price"));
	
			// 解析Book1的子节点
			TiXmlElement* Description = pItem->FirstChildElement("Description");
			printf("Description = %s\n", Description->GetText());
	
			TiXmlElement *Page = pItem->FirstChildElement("Page");
			printf("Page = %s\n", Page->GetText());
	
			printf("\n\n");
		}
	}
}

1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
19
20
21
22
23
24
25
26
27
28
29
30
31
32
33
34
35
36
37
38
39
40
41
42
43
44
45
46
47
48
49
50
51
52
53
54
55
56
57
58
59
60
61
62
63
64
七、总结
到此，使用tinyxml操作xml文件的教程已经完毕了，相信学到这里的童鞋们应该会如何使用了，那快做项目去吧！

代码中使用了很多指针，但是都没有进行释放，为什么呢？
我在网上找过了，网上都说，类结束后会自动释放，不知真假！
————————————————

                            版权声明：本文为博主原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接和本声明。

原文链接：https://blog.csdn.net/cpp_learner/article/details/122286597