# markdown convert to html (markdown 转化成 html 文件) 
[toc]

## 缘起
有项目需要用到markdown to html的功能，在GitHub上找了不少项目，经过测试，主要有两个问题：
1. 语法支持不全，定制不灵活。
2. 性能不好。
于是决定自己开发一个。

## target:
- 能支持完整的markdown语法（以GitHub为蓝本）
- 扩展和添加功能要灵活，保持:open-close的原则：对修改关闭，对扩展开放。
- 性能要高，1 Mb 的markdown文件，处理时间小于1秒。1 Mb/second
- 支持css 定制

## 使用方法
方法很简单：
```shell
$: git clone https://github.com/KunlunMaster/mdhtml.git
$: cd mdhtml
$: make 
$: ./test 
```

##  开发日志 
### 2019.12.16

- 完成task list 

### 2019.12.15

- 完成图片链接识别。可以识别图片链接与链接的重合。
- 完成行内`<code>`的识别
- 加入"default.css" 做基本的处理。

### 2019.12.13 

- 完成大体框架，性能和功能都不错。
- 还有一些功能需要完善：
  1. check list没做
  2. 多个[toc]没考虑
  3. 单行code细节没处理。
  4. css细节没处理。
  5. 单行识别性能没优化。
  6. 多线程没考虑。
  7. default block的细节没做
  8. 流程图和公式识别没做。

