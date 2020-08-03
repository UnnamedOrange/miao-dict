# system

系统（`system`）类属于命名空间 `miao::core`，表示整个系统。程序运行时，只允许存在一个系统实例。

系统类应该支持仅在内存中存在，无需文件的支持。只有在需要保存和读档时，才应该使用文件接口。

## 工作目录

程序的所有文件均保存在工作目录（`working_dir`）下的 `miao_dict` 文件夹中。默认工作目录为当前目录。 

目录结构。

```
working_dir
|--miao_dict
|  |--library
|  |  |--0                   # local
|  |  |  |--items
|  |  |  |  |--0.json
|  |  |  |  |--...
|  |  |  |--passages
|  |  |  |  |--0.json
|  |  |  |  |--...
|  |  |  |--pronunciations
|  |  |  |  |--...           # TODO
|  |  |  |--raws.json
|  |  |  |--raw_items.json
|  |  |  |--library.json
|  |  |--1                   # others
|  |  |  |--...
|  |--sentence
|  |  |--0.json
|  |  |--1.json
```

