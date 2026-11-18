# 版本限制

仅限于1.9.x及以下

# license无需激活破解

1. 打开\Typora\resources\page-dist\static\js\LicenseIndex.180dd4c7.5c394f9a.chunk.js
2. 将搜索到的位置的

```c
【e.hasActivated="true"==】 后面添加 【"true",】 即：e.hasActivated="true"=="true",
```

# Typora去掉弹出的激活框

1. 找到文件Typora/resources/page-dist/license.html文件

2. 用记事本或者是vscode等打开在代码的倒数第二行加上

   ```html
     <script>
       setTimeout(() => {
         window.close();
       }, 1000); // 等页面加载完一会儿再关闭
     </script>
   ```

   