# 女少口阿

## 核心概念

### 库（library）

库由以下部分组成。

#### id

#### 标签（tag）

字符串，是这个库的名字。

#### 预设语言类型（lang）

字符串，预设值有：`zh`、`en`、`ja`、`custom`。

#### 词典（dict）

##### 词（item）

一个词（item）包含以下属性：

- id。
- 一般式（origin），字符串。
- 变体（variants），字符串的列表。
- 注音（notations），字符串的列表。
- 翻译（translates），列表，每个元素形如 `(lib_id, tag, meaning)`。
- 发音（pronunciation），发音 id 的列表。
- 句子（sentences），句子 id 的列表。

##### 未处理过的词（raw_item）

一个未处理过词（raw_item）包含以下属性：

- 一般式（origin），字符串。
- 出现次数（frequency），整数，用作排序依据。

#### 原式语料库（raw）

##### 片段（passage）

一个长字符串。可以使用程序辅助从片段中提取句子（sencence）或未处理过的词（raw_item）。

#### 句子库（sentences）

##### 句子（sentence）

一个句子包含以下属性：

- id。注意这个 id 是全局的 id，对于意思相同的句子，应当在不同的库中共享一个 id。
- 句子内容（content），字符串。

#### 语音库（pronunciations）

##### 语音数据

一个语音数据包含 id 和内容。

##### 语音引擎

对于满足特定预设语言标签的语言，支持使用特定方式直接生成语音，无需额外语音数据。目前暂不支持。

