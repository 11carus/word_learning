# WordFlow 核心参数表

本文档汇总 WordFlow 中会影响复习调度、单轮学习行为、数据状态和 PDF 显示效果的主要参数。

## 1. 复习调度输入参数

| 参数 | 类型 | 默认值/范围 | 单位 | 作用 | 所在位置 |
| --- | --- | --- | --- | --- | --- |
| `rating` | `ReviewRating` | Again、Hard、Good、Easy | — | 表示用户本次掌握程度，决定采用哪种间隔更新策略 | `reviewscheduler.h` |
| `previousIntervalDays` | `int` | 大于等于 0 | 天 | 上一次复习间隔，是计算新间隔和近似记忆稳定性的基础 | `reviewscheduler.cpp` |
| `previousEaseFactor` | `double` | 1.3～3.0 | — | 历史难度因子；越大，答对后的间隔增长越快 | `reviewscheduler.cpp` |
| `previousLapseCount` | `int` | 大于等于 0 | 次 | 计算前累计选择 Again 的次数 | `reviewscheduler.cpp` |
| `lastReviewDate` | `QDate` | 有效日期或空日期 | 日期 | 用于计算距上次复习实际经过的天数 | `reviewscheduler.cpp` |
| `today` | `QDate` | `QDate::currentDate()` | 日期 | 本次调度的基准日期，用于计算经过天数和下次复习日期 | `reviewscheduler.h` |

## 2. 复习算法常量

| 常量 | 当前值 | 作用 | 数值增大后的影响 | 所在位置 |
| --- | ---: | --- | --- | --- |
| `kDefaultEaseFactor` | 2.3 | 新单词或无效数据使用的默认难度因子 | 答对后间隔整体增长更快 | `reviewscheduler.cpp` |
| `kMinimumEaseFactor` | 1.3 | 难度因子的下限 | 困难单词的最小增长速度提高 | `reviewscheduler.cpp` |
| `kMaximumEaseFactor` | 3.0 | 难度因子的上限 | 容易单词允许获得更大的增长倍率 | `reviewscheduler.cpp` |

## 3. 记忆模型中间参数

| 参数 | 计算方式 | 范围 | 作用 | 是否写入数据库 |
| --- | --- | --- | --- | --- |
| `previousInterval` | `max(0, previousIntervalDays)` | 大于等于 0 | 对历史间隔进行容错处理 | 否 |
| `elapsedDays` | `lastReviewDate.daysTo(today)` | 大于等于 0 | 表示距上次复习实际经过的天数 | 否 |
| `stability` | `max(1, previousInterval)` | 大于等于 1 | 记忆稳定性，当前用历史间隔近似 | 否 |
| `retrievability` | `exp(-elapsedDays / stability)` | 0～1 | 估计用户当前还能记住单词的概率 | 否 |
| `successfulDelayBonus` | `1 - retrievability` | 0～1 | 延迟后仍答对时，用于增加下一次间隔 | 否 |

记忆保持率公式：

```text
retrievability = exp(-elapsedDays / stability)
```

## 4. 评分调度参数

### 4.1 新单词固定间隔

当 `previousIntervalDays == 0` 时使用以下参数：

| 评分 | 新间隔 | 难度因子变化 | 遗忘次数变化 | 含义 |
| --- | ---: | ---: | ---: | --- |
| Again | 1 天 | -0.20 | +1 | 完全忘记，很快再次复习 |
| Hard | 2 天 | -0.15 | 0 | 回忆困难，采用较短间隔 |
| Good | 3 天 | +0.05 | 0 | 正常记住，采用正常间隔 |
| Easy | 5 天 | +0.15 | 0 | 轻松记住，采用较长间隔 |

### 4.2 已学习单词增长参数

设：

- `I`：上一次复习间隔；
- `EF`：难度因子；
- `B`：延迟奖励 `successfulDelayBonus`。

| 评分 | 新间隔计算 | 基础系数 | 延迟奖励权重 | 最小增长 |
| --- | --- | ---: | ---: | ---: |
| Again | `1` | — | — | 重置为 1 天 |
| Hard | `round(I × (1.20 + 0.10 × B))` | 1.20 | 0.10 | 至少增加 1 天 |
| Good | `round(I × (EF + 0.30 × B))` | `EF` | 0.30 | 至少增加 1 天 |
| Easy | `round(I × (EF + 0.80 + 0.40 × B))` | `EF + 0.80` | 0.40 | 至少增加 2 天 |

最终结果：

```text
nextReviewDate = today + intervalDays
```

## 5. 单轮复习行为参数

| 参数 | 类型 | 当前值 | 作用 | 数值增大后的影响 | 所在位置 |
| --- | --- | ---: | --- | --- | --- |
| `kMaximumSessionAttempts` | `int` | 3 | 限制一个单词在同一轮中的最大出现次数 | 即时强化更充分，但单轮任务更长 | `mainwindow.h` |
| `m_sessionReviewAttempts` | `QHash<int, int>` | 运行时生成 | 记录每个单词在本轮已经出现的次数 | — | `mainwindow.h` |
| `m_currentReviewIndex` | `int` | 初始为 -1 | 指向当前复习队列位置；-1 表示无任务 | — | `mainwindow.h` |

当前重新入队规则：

| 评分 | 是否重新加入本轮队尾 |
| --- | --- |
| Again | 是，未达到 3 次时 |
| Hard | 是，未达到 3 次时 |
| Good | 是，未达到 3 次时 |
| Easy | 否 |

## 6. 数据库学习状态参数

| 字段 | 数据库类型 | 默认值 | 作用 | 是否直接影响调度 |
| --- | --- | --- | --- | --- |
| `next_review_date` | `TEXT` | 添加单词当天 | 决定单词何时进入复习队列 | 是 |
| `review_count` | `INTEGER` | 0 | 记录累计复习次数，用于统计 | 否 |
| `interval_days` | `INTEGER` | 0 | 保存当前复习间隔；0 表示新单词 | 是 |
| `ease_factor` | `REAL` | 2.3 | 保存当前难度因子 | 是 |
| `lapse_count` | `INTEGER` | 0 | 保存累计遗忘次数 | 当前不直接影响公式 |
| `last_review_date` | `TEXT` | `NULL` | 保存最近一次实际复习日期 | 是 |

到期筛选条件：

```sql
WHERE next_review_date <= :date
```

到期队列排序规则：

```sql
ORDER BY next_review_date ASC, word COLLATE NOCASE ASC
```

## 7. PDF 显示参数

| 参数 | 类型 | 当前值 | 单位 | 作用 | 数值增大后的影响 |
| --- | --- | ---: | --- | --- | --- |
| `kPageMargin` | `int` | 12 | 逻辑像素 | PDF 页面与视口之间的边距 | 页面四周留白增大 |
| `kWheelScrollMultiplier` | `int` | 2 | 倍 | 鼠标滚轮滚动速度系数 | 页面滚动更快 |
| `kMinimumZoomFactor` | `qreal` | 0.5 | 倍 | 限制最小缩放比例 | 允许的最小页面尺寸增大 |
| `kMaximumZoomFactor` | `qreal` | 3.0 | 倍 | 限制最大缩放比例 | 允许进一步放大页面 |
| `m_zoomFactor` | `qreal` | 1.0 | 倍 | 用户当前相对于适宽比例的缩放倍数 | 页面显示更大 |
| `devicePixelRatio` | `qreal` | 由屏幕决定 | 倍 | 计算高 DPI 屏幕的实际渲染尺寸 | 渲染图像更清晰，但占用更多内存 |

## 8. 最核心参数摘要

| 优先级 | 参数 | 影响对象 | 重要性说明 |
| ---: | --- | --- | --- |
| 1 | `rating` | 调度策略 | 直接决定 Again、Hard、Good 或 Easy 分支 |
| 2 | `previousIntervalDays` | 新间隔 | 是所有间隔增长计算的基础 |
| 3 | `easeFactor` | 增长倍率 | 决定 Good 和 Easy 后间隔增长速度 |
| 4 | `lastReviewDate`、`today` | 记忆保持率 | 决定实际经过天数和延迟奖励 |
| 5 | `kMaximumSessionAttempts` | 单轮学习强度 | 决定未完全掌握的单词最多重复几次 |
| 6 | `next_review_date` | 每日任务 | 决定单词是否进入今日复习队列 |
