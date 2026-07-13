# 基于 Qt/C++ 的离线背单词软件课程项目任务书

# Course Project Brief: Offline Vocabulary Learning Software Based on Qt/C++

---

## 一、项目基本信息

## 1. Project Information

| 项目   | 中文              | English                              |
| ---- | --------------- | ------------------------------------ |
| 项目名称 | 离线背单词软件         | Offline Vocabulary Learning Software |
| 项目类型 | C++课程项目         | C++ Course Project                   |
| 开发周期 | 4天              | 4 Days                               |
| 开发语言 | C++             | C++                                  |
| 开发框架 | Qt 6、Qt Widgets | Qt 6 and Qt Widgets                  |
| 数据存储 | SQLite本地数据库     | Local SQLite Database                |
| 运行平台 | Windows         | Windows                              |

> 为保证4天内完成，本项目采用 Qt Widgets，不使用复杂的 QML、云同步和完整 FSRS 算法。

---

## 二、项目背景

## 2. Project Background

### 中文

传统单词本主要依靠人工记录和固定顺序复习，不能根据用户对单词的掌握情况安排学习任务。本项目拟开发一款简单的离线背单词软件，实现单词录入、卡片复习、复习评价和本地数据保存等基本功能。

该项目主要用于练习 C++程序设计、Qt图形界面开发、SQLite数据库操作和简单的软件工程流程。

### English

Traditional vocabulary notebooks rely on manual recording and fixed review orders. This project aims to develop a simple offline vocabulary learning application with basic functions such as vocabulary entry, card review, review ratings, and local data storage.

The project is intended to practise C++ programming, Qt graphical interface development, SQLite database operations, and basic software engineering procedures.

---

## 三、项目目标

## 3. Project Objectives

### 中文

本项目需要完成以下目标：

1. 建立一个可以正常运行的 Qt 桌面程序。
2. 实现单词的添加、修改、删除和查询。
3. 使用卡片形式显示单词和释义。
4. 支持“忘记、困难、记住、简单”四种评价。
5. 根据评价结果简单调整下次复习日期。
6. 使用 SQLite 保存单词和复习记录。
7. 显示简单的学习统计数据。

### English

The project shall achieve the following objectives:

1. Build a functional Qt desktop application.
2. Add, edit, delete, and search vocabulary entries.
3. Display vocabulary using flashcards.
4. Support four ratings: Again, Hard, Good, and Easy.
5. Adjust the next review date according to the rating.
6. Store vocabulary and review records in SQLite.
7. Display basic learning statistics.

---

## 四、功能需求

## 4. Functional Requirements

### 4.1 单词管理

### 4.1 Vocabulary Management

每个单词包含以下信息：

| 字段     | English Field    | 说明        |
| ------ | ---------------- | --------- |
| 单词     | Word             | 英文单词或短语   |
| 释义     | Definition       | 中文释义      |
| 例句     | Example          | 简单英文例句    |
| 熟悉程度   | Familiarity      | 当前学习状态    |
| 下次复习日期 | Next Review Date | 系统计算的复习日期 |

用户可以完成以下操作：

* 添加单词
* 修改单词
* 删除单词
* 搜索单词
* 查看全部单词

The user shall be able to:

* Add vocabulary
* Edit vocabulary
* Delete vocabulary
* Search vocabulary
* View all vocabulary entries

---

### 4.2 单词复习

### 4.2 Vocabulary Review

复习页面首先显示单词：

```text
abandon
```

用户点击“显示答案”后，显示：

```text
释义：放弃；遗弃
例句：He abandoned the plan.
```

复习评价包括：

| 中文 | English | 处理方式  |
| -- | ------- | ----- |
| 忘记 | Again   | 1天后复习 |
| 困难 | Hard    | 2天后复习 |
| 记住 | Good    | 4天后复习 |
| 简单 | Easy    | 7天后复习 |

评价完成后，系统保存复习结果并显示下一张单词卡片。

After a rating is selected, the system shall save the result and display the next card.

---

### 4.3 今日复习

### 4.3 Daily Review

系统应查询满足以下条件的单词：

```text
下次复习日期 ≤ 当前日期
```

The system shall retrieve vocabulary whose:

```text
Next review date ≤ Current date
```

首页显示：

* 单词总数
* 今日待复习数量
* 今日已复习数量
* 开始复习按钮

The home page shall display:

* Total vocabulary count
* Number of words due today
* Number of words reviewed today
* Start Review button

---

### 4.4 简单统计

### 4.4 Basic Statistics

系统只需显示以下统计数据：

* 单词总数
* 今日复习数量
* 累计复习次数
* Again、Hard、Good、Easy 各评价次数

The system shall display:

* Total number of vocabulary entries
* Reviews completed today
* Total number of reviews
* Number of Again, Hard, Good, and Easy ratings

---

## 五、非功能需求

## 5. Non-Functional Requirements

### 中文

1. 软件应能够在 Windows 系统正常运行。
2. 软件应在无网络环境下使用。
3. 软件关闭后，已保存的数据不能丢失。
4. 界面应简单清晰，操作步骤不宜过多。
5. 删除单词前应显示确认提示。
6. 数据库操作失败时应显示错误信息。
7. 代码结构应基本清晰，避免将全部功能写在一个文件中。

### English

1. The application shall run on Windows.
2. The application shall work without an internet connection.
3. Saved data shall remain available after restarting the application.
4. The interface shall be simple and easy to use.
5. A confirmation message shall be displayed before deleting vocabulary.
6. Database errors shall be reported clearly.
7. The code shall be divided into basic modules instead of being placed in one file.

---

## 六、系统概要设计

## 6. System Design

### 6.1 系统结构

### 6.1 System Architecture

```text
┌─────────────────────────────┐
│       Qt Widgets 界面        │
│ 首页、单词管理、复习、统计    │
└──────────────┬──────────────┘
               │
┌──────────────▼──────────────┐
│        业务逻辑模块          │
│ 单词管理、复习日期计算        │
└──────────────┬──────────────┘
               │
┌──────────────▼──────────────┐
│       SQLite 数据库          │
│ 单词表、复习记录表            │
└─────────────────────────────┘
```

```text
┌─────────────────────────────┐
│       Qt Widgets UI         │
│ Home, Vocabulary, Review    │
└──────────────┬──────────────┘
               │
┌──────────────▼──────────────┐
│       Business Logic        │
│ Management and Scheduling   │
└──────────────┬──────────────┘
               │
┌──────────────▼──────────────┐
│       SQLite Database       │
│ Vocabulary and Review Logs  │
└─────────────────────────────┘
```

---

### 6.2 主要界面

### 6.2 Main Interfaces

| 页面    | 主要功能          | Main Functions                |
| ----- | ------------- | ----------------------------- |
| 首页    | 显示统计信息和开始复习按钮 | Statistics and Start Review   |
| 单词管理页 | 添加、修改、删除、查询单词 | Add, edit, delete, and search |
| 复习页   | 显示卡片和评价按钮     | Display cards and ratings     |
| 统计页   | 显示简单学习数据      | Display basic learning data   |

---

## 七、数据库设计

## 7. Database Design

### 7.1 单词表 `words`

| 字段               | 类型      | 说明     |
| ---------------- | ------- | ------ |
| id               | INTEGER | 主键     |
| word             | TEXT    | 单词     |
| definition       | TEXT    | 释义     |
| example          | TEXT    | 例句     |
| next_review_date | TEXT    | 下次复习日期 |
| review_count     | INTEGER | 复习次数   |
| created_at       | TEXT    | 创建时间   |

### 7.2 复习记录表 `review_logs`

| 字段               | 类型      | 说明     |
| ---------------- | ------- | ------ |
| id               | INTEGER | 主键     |
| word_id          | INTEGER | 对应单词   |
| rating           | INTEGER | 评价结果   |
| review_date      | TEXT    | 复习时间   |
| next_review_date | TEXT    | 下次复习日期 |

数据关系：

```text
Word 1 ───── N ReviewLog
```

---

## 八、程序设计

## 8. Program Design

项目可以采用以下简单目录：

```text
VocabularyApp/
├── CMakeLists.txt
├── main.cpp
├── mainwindow.h
├── mainwindow.cpp
├── mainwindow.ui
├── database/
│   ├── databasemanager.h
│   └── databasemanager.cpp
├── review/
│   ├── reviewscheduler.h
│   └── reviewscheduler.cpp
└── README.md
```

主要类包括：

| 类名                | 主要职责         |
| ----------------- | ------------ |
| `MainWindow`      | 管理界面和页面切换    |
| `DatabaseManager` | 连接数据库并执行增删改查 |
| `ReviewScheduler` | 根据评价计算下次复习日期 |

简单排程逻辑：

```cpp
Again -> 当前日期加1天
Hard  -> 当前日期加2天
Good  -> 当前日期加4天
Easy  -> 当前日期加7天
```

本课程项目不要求实现复杂的 FSRS 算法。

---

## 九、四天开发计划

## 9. Four-Day Development Plan

### 第一天：计划、界面和数据库

### Day 1: Planning, Interface, and Database

完成内容：

* 创建 Qt/CMake 项目
* 设计主窗口
* 创建首页和单词管理页面
* 创建 SQLite 数据库
* 创建 `words` 和 `review_logs` 表
* 完成数据库连接测试

Deliverables:

* Qt project structure
* Main window
* Basic pages
* SQLite database
* Database connection test

---

### 第二天：单词管理

### Day 2: Vocabulary Management

完成内容：

* 添加单词
* 修改单词
* 删除单词
* 查询和显示单词
* 搜索单词
* 删除确认提示

Deliverables:

* Vocabulary CRUD functions
* Vocabulary list
* Search function
* Delete confirmation

---

### 第三天：复习功能

### Day 3: Review Function

完成内容：

* 查询今日待复习单词
* 显示单词卡片
* 显示答案
* 实现四种评价
* 计算下次复习日期
* 保存复习记录
* 自动切换下一张卡片

Deliverables:

* Review page
* Four rating buttons
* Simple scheduling
* Review record storage

---

### 第四天：统计、测试和整理

### Day 4: Statistics, Testing, and Documentation

完成内容：

* 显示单词总数
* 显示今日复习数量
* 显示评价次数
* 测试主要功能
* 修复程序错误
* 编写 README
* 整理项目报告和运行截图

Deliverables:

* Basic statistics
* Tested application
* README
* Project report
* Screenshots

---

## 十、测试内容

## 10. Testing Requirements

| 编号  | 测试内容     | 预期结果        |
| --- | -------- | ----------- |
| T01 | 添加单词     | 单词成功保存并显示   |
| T02 | 修改单词     | 修改后的内容正确显示  |
| T03 | 删除单词     | 确认后单词被删除    |
| T04 | 搜索单词     | 显示符合条件的单词   |
| T05 | 点击显示答案   | 正确显示释义和例句   |
| T06 | 点击 Again | 下次复习日期增加1天  |
| T07 | 点击 Good  | 下次复习日期增加4天  |
| T08 | 完成复习     | 生成一条复习记录    |
| T09 | 重启软件     | 原有数据仍然存在    |
| T10 | 无待复习单词   | 显示“今日任务已完成” |

---

## 十一、项目成果

## 11. Deliverables

课程项目完成后需要提交：

1. Qt/C++完整源代码；
2. CMake构建文件；
3. SQLite数据库文件；
4. 可执行程序；
5. 项目任务书；
6. 简单设计说明；
7. 测试结果；
8. 软件运行截图；
9. README使用说明。

The project deliverables shall include:

1. Complete Qt/C++ source code;
2. CMake build file;
3. SQLite database file;
4. Executable application;
5. Project brief;
6. Basic design document;
7. Test results;
8. Application screenshots;
9. README documentation.

---

## 十二、验收标准

## 12. Acceptance Criteria

满足以下条件即可认为课程项目完成：

* 程序能够正常编译和启动；
* 可以添加、修改、删除和搜索单词；
* 可以显示单词卡片和答案；
* 可以选择四种复习评价；
* 系统可以计算下次复习日期；
* 每次复习可以保存记录；
* 首页能够显示基本统计信息；
* 软件重启后数据仍然存在；
* 软件核心功能可以离线运行；
* 项目能够在4天内完成并进行演示。

The course project is considered complete when:

* The application compiles and starts correctly;
* Vocabulary can be added, edited, deleted, and searched;
* Flashcards and answers can be displayed;
* Four review ratings are available;
* The next review date can be calculated;
* Review records can be saved;
* Basic statistics are displayed;
* Data remains after restarting;
* Core functions work offline;
* The project can be demonstrated within the four-day schedule.
