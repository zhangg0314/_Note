# 换行符

```python
#\在python中可以用于一条语句写多行
s = "这是一" \
    "条" \
    "语句"
print(s)
```

# List列表

访问。[a:b]，打印的值不包括list[b]

# Tuple元组

元组不支持修改，一旦赋值就不能二次赋值。

单个元组表示要加逗号，即tuple = (12,)

# Dict字典

## 访问

- dict[key]
  ```pyt
  print(dict[key])#访问指定单个键的单个值
  ```

- dict.keys()
  ```py
  print(dict,keys())#访问所有键
  ```

- dict.values()

  ```python
  print(dict.values())#访问所有值
  ```

- dict

  ```python
  print(dict)#访问整个字典
  ```

# 数据类型转换

## 获取变量类型

```python
a = 10
print(type(a))

#输出
<class 'int'>
```

## 各种类型转换函数

![image-20250415205617992](..\figure\image-20250415205617992.png)

# 标识符和关键字

```python
import  keyword
keyword.kwlist	 #查看python所有内置的标识符

#输出
['False', 'None', 'True', 'and', 'as', 'assert', 'async', 'await', 'break', 'class', 'continue', 'def', 'del', 'elif', 'else', 'except', 'finally', 'for', 'from', 'global', 'if', 'import', 'in', 'is', 'lambda', 'nonlocal', 'not', 'or', 'pass', 'raise', 'return', 'try', 'while', 'with', 'yield']
```

# 输出输入

## 输出

### 普通输出

```python
print("hello,world")#默认带换行
```

### 格式化输出

```python
name = "小明"
age = 18
print("我是%s,我今年%d岁" %(name,age))
```

## 输入

```python
#python2/3中的共同输入
a = input("请输入")
print(type(a))#输入函数永远会被转为字符串
a = int(a)
print(type(a))

#输出
<class 'str'>
<class 'int'>
```

# 运算符

## 算术运算符

`加减乘除余、整除-// 、`

## 关系运算符

`==、!= <> 、>、 <、 >= 、<=`

## 赋值运算符

`**= - 幂赋值:`

```python
a = 3
a = a**3
print(a) #输出27
```

## 逻辑运算符

`and or not`

```python
a = 10
print(bool(a and 2))
print(bool(a and 0))

#输出
True
False
```

# 常用语句

## if判断语句

```python
a = 15
if a == 10:#注意冒号
    print("a == 10")#注意缩进
    a += 5
    if a == 15: #if嵌套
        print("a == 15")
elif a == 15:
	print("a == 15")
else:
    print("a != 10")
```

## 循环语句

### while循环

```python
while 1:#注意分号
    print("haaha")#注意缩进
```

### for循环

```python
a = "hello world"
for i in a:
	print(i)
#输出
h
e
l
l
o
 
w
o
r
l
d

for i in range(1,3):#不包括3
	print(i)
```

### break

终止循环

### continue

终止本次循环

# 常用模块

## 随机数生成

```python
import random
a = ndom.randint(0,2)#从0~2随机取一个整数，包含2
```

