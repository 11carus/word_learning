# WordFlow 核心文件与算法说明

本文档用于帮助开发者快速了解 WordFlow 的主要代码结构、核心算法及关键变量。

## 1. 核心文件概览

| 重要程度 | 文件 | 主要职责 |
| --- | --- | --- |
| 1 | `src/review/reviewscheduler.cpp` | 计算记忆保持率、复习间隔和下次复习日期 |
| 2 | `src/mainwindow.cpp` | 串联界面、复习流程、数据库、词典和 PDF 模块 |
| 3 | `src/database/databasemanager.cpp` | 管理单词、复习日志、到期查询和学习统计 |
| 4 | `src/dictionary/dictionarymanager.cpp` | 导入 ECDICT CSV，并提供离线释义查询 |
| 5 | `src/pdf/pdfselectableview.cpp` | 渲染 PDF、处理缩放、坐标映射和拖拽选词 |

对应头文件负责声明数据结构、类接口和成员状态：

- `src/review/reviewscheduler.h`
- `src/mainwindow.h`
- `src/database/databasemanager.h`
- `src/dictionary/dictionarymanager.h`
- `src/pdf/pdfselectableview.h`

`src/main.cpp` 只负责创建应用程序和显示主窗口，不包含核心业务算法。

## 2. 自适应间隔复习算法

实现位置：`src/review/reviewscheduler.cpp`

入口函数：

```cpp
ReviewSchedule ReviewScheduler::schedule(
    ReviewRating rating,
    int previousIntervalDays,
    double previousEaseFactor,
    int previousLapseCount,
    const QDate &lastReviewDate,
    const QDate &today);
```

### 2.1 输入参数

| 参数 | 含义 |
| --- | --- |
| `rating` | 用户本次评分：Again、Hard、Good 或 Easy |
| `previousIntervalDays` | 上一次安排的复习间隔，单位为天；0 表示新单词 |
| `previousEaseFactor` | 历史难度因子，数值越大，答对后间隔增长越快 |
| `previousLapseCount` | 该单词累计选择 Again 的次数 |
| `lastReviewDate` | 上一次实际复习日期 |
| `today` | 本次调度计算的基准日期，默认是系统当前日期 |

### 2.2 输出结果

`ReviewSchedule` 包含：

| 字段 | 含义 |
| --- | --- |
| `nextReviewDate` | 下次复习日期 |
| `intervalDays` | 新的复习间隔，单位为天 |
| `easeFactor` | 更新后的难度因子 |
| `lapseCount` | 更新后的累计遗忘次数 |
| `retrievability` | 当前记忆保持率，范围为 0 到 1 |

### 2.3 记忆保持率

算法使用指数遗忘曲线估计当前记忆保持率：

```text
R = exp(-t / S)
```

其中：

- `R` 对应 `retrievability`，表示当前记忆保持率；
- `t` 对应 `elapsedDays`，表示距上次复习实际经过的天数；
- `S` 对应 `stability`，当前使用上一次复习间隔近似表示记忆稳定性。

实际经过时间越长，保持率越低。算法进一步计算：

```text
successfulDelayBonus = 1 - retrievability
```

如果用户在较低保持率下仍能答对，算法会适当放大下一次复习间隔。

### 2.4 新单词初始间隔

当 `previousIntervalDays == 0` 时，使用固定初始间隔：

| 评分 | 新间隔 | 难度因子变化 |
| --- | ---: | ---: |
| Again | 1 天 | -0.20 |
| Hard | 2 天 | -0.15 |
| Good | 3 天 | +0.05 |
| Easy | 5 天 | +0.15 |

### 2.5 已学习单词的间隔更新

设：

- `I` 为上一次复习间隔；
- `EF` 为难度因子；
- `B` 为延迟奖励 `successfulDelayBonus`。

不同评分采用不同策略：

```text
Again: newInterval = 1
Hard : newInterval = max(I + 1, round(I × (1.20 + 0.10 × B)))
Good : newInterval = max(I + 1, round(I × (EF + 0.30 × B)))
Easy : newInterval = max(I + 2, round(I × (EF + 0.80 + 0.40 × B)))
```

Again 会将 `lapseCount` 加 1。难度因子最终限制在 `[1.3, 3.0]`，防止间隔增长过慢或过快。

下次复习日期为：

```text
nextReviewDate = today + intervalDays
```

该算法每次调度只进行固定数量的计算，时间复杂度和额外空间复杂度均为 `O(1)`。

## 3. 到期单词筛选与排序

实现位置：`DatabaseManager::dueWords()`。

核心查询条件：

```sql
WHERE next_review_date <= :date
ORDER BY next_review_date ASC, word COLLATE NOCASE ASC
```

规则如下：

1. 只加载今天及以前到期的单词；
2. 先处理逾期最久的单词；
3. 到期日期相同时，按忽略大小写的字母顺序排列。

每次复习完成后，`recordReview()` 使用数据库事务同时完成两项操作：

1. 向 `review_logs` 写入本次评分；
2. 更新 `words` 中的间隔、难度因子、遗忘次数和下次复习日期。

事务保证两项操作同时成功或同时回滚，避免数据不一致。

## 4. 本轮重复复习队列

实现位置：`MainWindow::rateCurrentWord()`。

相关成员变量：

| 变量 | 作用 |
| --- | --- |
| `m_dueWords` | 当前复习队列 |
| `m_sessionReviewAttempts` | 记录每个单词在本轮已经出现的次数 |
| `m_currentReviewIndex` | 当前单词在队列中的索引 |
| `kMaximumSessionAttempts` | 单词在一轮中的最大出现次数，当前为 3 |

处理规则：

- 选择 Easy：该单词本轮不再出现；
- 选择 Again、Hard 或 Good：如果未达到次数上限，则将更新后的单词追加到队尾；
- 达到 3 次后不再入队，避免任务无限循环。

例如，初始队列为：

```text
[A, B, C]
```

复习 A 后选择 Again，队列变为：

```text
[A, B, C, A]
```

程序继续向后移动，因此接下来的显示顺序是 `B -> C -> A`。

## 5. ECDICT CSV 状态机解析

实现位置：`DictionaryManager::importEcdictCsv()`。

不能直接使用 `split(',')` 解析 CSV，因为释义字段本身可能包含逗号或引号，例如：

```csv
apple,"苹果,苹果树"
quote,"包含""引号""的释义"
```

解析器使用以下状态变量：

| 变量 | 作用 |
| --- | --- |
| `field` | 当前正在解析的字段 |
| `record` | 当前一行已经解析出的字段列表 |
| `columns` | 表头名称到列号的映射 |
| `inQuotes` | 当前是否位于双引号字段内 |
| `headerRead` | 是否已经解析第一行表头 |
| `importedCount` | 成功导入的词条数量 |
| `failed` | 是否发生 SQL 插入错误 |

状态转换规则：

- 普通状态遇到逗号：结束当前字段；
- 普通状态遇到换行：结束当前记录；
- 普通状态遇到双引号：进入引号状态；
- 引号状态中的逗号和换行：作为字段内容；
- 引号状态中连续两个双引号：解析为一个普通双引号。

文件只需从头到尾扫描一次，因此时间复杂度为 `O(n)`，其中 `n` 为 CSV 字符总数。

## 6. PDF 缩放与坐标映射

实现位置：`PdfSelectableView`。

### 6.1 页面缩放

首先计算适合窗口宽度的比例：

```text
fitScale = availableWidth / pdfPageWidth
```

再叠加用户缩放倍数：

```text
scale = fitScale × zoomFactor
```

渲染时再乘以设备像素比 `devicePixelRatio`，保证高 DPI 屏幕上的显示清晰度。

### 6.2 坐标转换

鼠标事件使用视图坐标，而 Qt PDF 的文字选择使用 PDF 页面坐标，因此需要双向映射。

视图坐标转换为 PDF 页面坐标：

```text
pageX = (viewX - pageLeft) × pdfWidth / displayedWidth
pageY = (viewY - pageTop)  × pdfHeight / displayedHeight
```

PDF 页面坐标转换为视图坐标：

```text
viewX = pageLeft + pageX × displayedWidth / pdfWidth
viewY = pageTop  + pageY × displayedHeight / pdfHeight
```

完整选词流程：

```text
鼠标拖拽
  -> 将视图坐标转换为 PDF 坐标
  -> 调用 QPdfDocument::getSelection()
  -> 得到文本和选区边界
  -> 将边界转换回视图坐标
  -> 绘制选区高亮
```

## 7. 主要数据结构

`WordEntry` 定义在 `src/database/databasemanager.h`，保存一个单词的完整学习状态：

| 字段 | 含义 |
| --- | --- |
| `id` | 数据库主键 |
| `word` | 英文单词或词组 |
| `definition` | 单词释义 |
| `example` | 可选例句 |
| `source` | 单词来源 |
| `nextReviewDate` | 下次复习日期 |
| `reviewCount` | 累计复习次数 |
| `intervalDays` | 当前复习间隔 |
| `easeFactor` | 当前难度因子 |
| `lapseCount` | 累计遗忘次数 |
| `lastReviewDate` | 最近一次复习日期 |
| `createdAt` | 数据库记录创建时间 |

## 8. 阅读代码的建议顺序

建议按照以下顺序阅读项目：

1. 阅读 `reviewscheduler.h/.cpp`，理解复习算法；
2. 阅读 `databasemanager.h/.cpp`，理解学习状态如何保存；
3. 阅读 `mainwindow.cpp` 中的 `loadDueWords()` 和 `rateCurrentWord()`；
4. 阅读 `dictionarymanager.cpp`，理解离线词典导入；
5. 阅读 `pdfselectableview.cpp`，理解 PDF 渲染和选词。

复习主调用链如下：

```text
DatabaseManager::dueWords()
  -> MainWindow::showCurrentReviewWord()
  -> 用户显示答案并评分
  -> ReviewScheduler::schedule()
  -> DatabaseManager::recordReview()
  -> 必要时将单词重新追加到本轮队列
```
