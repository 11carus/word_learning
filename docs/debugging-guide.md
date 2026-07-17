# WordFlow Debug 调试指南

本文说明如何在 Windows、VS Code、MinGW GDB 环境下调试 WordFlow，包括 Debug 构建、断点设置、单步执行、变量观察和主要功能的调试流程。

> 文中行号对应当前版本源码。以后修改代码后行号可能变化，此时应按函数名定位。

## 1. 调试环境

当前项目使用：

- Qt 6.10.3 MinGW 64-bit
- MinGW GDB：`D:/Qt/Tools/mingw1310_64/bin/gdb.exe`
- CMake Debug 预设：`build/debug`
- 调试程序：`build/debug/wordflow.exe`
- VS Code 配置：`.vscode/launch.json`

VS Code 建议安装以下扩展：

- C/C++（Microsoft）
- CMake Tools（Microsoft）

## 2. 首次配置与编译

在 VS Code 中打开项目根目录，然后打开终端执行：

```powershell
qt-cmake --preset debug
cmake --build --preset debug
```

`debug` 预设会设置：

```text
CMAKE_BUILD_TYPE=Debug
```

Debug 构建包含调试符号，GDB 才能把机器指令映射回 `.cpp` 源码。

如果修改的只是 `.cpp` 或 `.h`，通常只需重新编译：

```powershell
cmake --build --preset debug
```

## 3. 添加和管理断点

### 3.1 添加普通断点

1. 打开需要调试的 `.cpp` 文件。
2. 在目标代码行左侧的行号区域单击。
3. 出现红色圆点表示断点已添加。
4. 再次单击红点可删除断点。

也可以把光标放在目标行并按 `F9` 切换断点。

断点应放在可执行语句上，不要放在空行、注释、函数声明或只有花括号的行上。

### 3.2 禁用断点

在左侧“运行和调试”面板的“断点”区域取消勾选，可以暂时禁用断点而不删除它。

### 3.3 条件断点

1. 右键单击已经添加的红色断点。
2. 选择“编辑断点”。
3. 选择“表达式”，输入条件。

示例：

```cpp
wordId == 1
rating == 1
previousIntervalDays > 30
```

`ReviewRating` 的数值为：

| 评分 | 数值 |
|---|---:|
| Again | 1 |
| Hard | 2 |
| Good | 3 |
| Easy | 4 |

条件断点适合在循环或频繁调用的函数中只拦截特定数据。

## 4. 推荐断点

### 4.1 程序启动

| 文件与行号 | 断点位置 | 用途 |
|---|---|---|
| `src/main.cpp:7` | `QApplication app(...)` | 检查程序入口 |
| `src/main.cpp:9` | `MainWindow window` | 进入主窗口构造过程 |
| `src/mainwindow.cpp:29` | `initializeDatabase()` | 检查界面创建后的数据库初始化 |
| `src/mainwindow.cpp:345` | `m_database.open(...)` | 检查学习数据库路径和打开结果 |
| `src/mainwindow.cpp:355` | `m_dictionary.open(...)` | 检查词典数据库初始化 |

重点观察：

```text
databasePath
dictionaryPath
m_database
m_dictionary
```

### 4.2 添加单词

| 文件与行号 | 断点位置 | 用途 |
|---|---|---|
| `src/mainwindow.cpp:425` | 输入检查 | 查看界面输入内容 |
| `src/mainwindow.cpp:430` | 调用 `addWord()` | 从界面进入数据库模块 |
| `src/database/databasemanager.cpp:156` | 绑定 SQL 参数 | 检查即将保存的数据 |
| `src/database/databasemanager.cpp:162` | `query.exec()` | 检查 SQL 是否执行成功 |

重点观察：

```text
word
definition
example
source
query
```

### 4.3 加载复习队列

| 文件与行号 | 断点位置 | 用途 |
|---|---|---|
| `src/mainwindow.cpp:529` | 查询到期单词 | 查看今日复习队列 |
| `src/mainwindow.cpp:534` | 设置当前索引 | 检查空队列与非空队列 |
| `src/mainwindow.cpp:553` | 取得当前单词 | 查看正在展示的 `WordEntry` |

重点观察：

```text
m_dueWords
m_dueWords.size()
m_currentReviewIndex
entry
```

### 4.4 复习评分与调度算法

| 文件与行号 | 断点位置 | 用途 |
|---|---|---|
| `src/mainwindow.cpp:583` | 取得当前复习单词 | 检查评分前状态 |
| `src/mainwindow.cpp:584` | 调用调度算法 | 使用“单步进入”进入 `schedule()` |
| `src/review/reviewscheduler.cpp:20` | 创建计算结果 | 调度算法入口 |
| `src/review/reviewscheduler.cpp:34` | 计算经过天数 | 检查遗忘曲线输入 |
| `src/review/reviewscheduler.cpp:38` | 计算保持率 | 检查 `retrievability` |
| `src/review/reviewscheduler.cpp:44` | 判断是否为新单词 | 区分首次与历史复习 |
| `src/mainwindow.cpp:590` | 保存调度结果 | 从算法进入数据库事务 |

重点观察：

```text
rating
previousIntervalDays
previousEaseFactor
previousLapseCount
elapsedDays
stability
result.retrievability
result.intervalDays
result.easeFactor
result.lapseCount
```

### 4.5 保存复习记录

| 文件与行号 | 断点位置 | 用途 |
|---|---|---|
| `src/database/databasemanager.cpp:212` | 开始事务 | 检查事务是否成功开启 |
| `src/database/databasemanager.cpp:226` | 插入复习日志 | 检查 `review_logs` 写入 |
| `src/database/databasemanager.cpp:250` | 更新单词状态 | 检查 `words` 更新行数 |
| `src/database/databasemanager.cpp:256` | 提交事务 | 检查两次写入是否一起成功 |

重点观察：

```text
wordId
rating
nextReviewDate
intervalDays
easeFactor
lapseCount
logQuery.lastError().text()
wordQuery.numRowsAffected()
```

### 4.6 ECDICT 词典

| 文件与行号 | 断点位置 | 用途 |
|---|---|---|
| `src/dictionary/dictionarymanager.cpp:68` | 打开 CSV | 检查导入文件路径 |
| `src/dictionary/dictionarymanager.cpp:74` | 读取 CSV | 检查编码和文件内容 |
| `src/dictionary/dictionarymanager.cpp:80` | 开始导入事务 | 检查词典数据库连接 |
| `src/dictionary/dictionarymanager.cpp:199` | 查询入口 | 检查待查询单词 |
| `src/dictionary/dictionarymanager.cpp:204` | 执行查询 | 检查是否命中词条 |

ECDICT 文件较大，不要在逐字符解析循环中设置普通断点，否则会频繁暂停。

### 4.7 PDF 选词

| 文件与行号 | 断点位置 | 用途 |
|---|---|---|
| `src/pdf/pdfselectableview.cpp:164` | 鼠标释放事件 | 检查是否完成拖选 |
| `src/pdf/pdfselectableview.cpp:171` | 更新最终选区 | 检查鼠标位置转换 |
| `src/pdf/pdfselectableview.cpp:173` | 发出选词信号 | 查看最终文本 |
| `src/pdf/pdfselectableview.cpp:273` | 计算 PDF 坐标 | 检查坐标映射 |
| `src/pdf/pdfselectableview.cpp:274` | 获取 PDF 选区 | 检查 `QPdfSelection` |

重点观察：

```text
m_currentPage
m_dragStart
position
clampedPosition
endPoint
m_selection
selectedText()
```

不建议在 `paintEvent()` 和 `mouseMoveEvent()` 中设置普通断点，因为它们触发非常频繁。

## 5. 启动调试

1. 确认需要的断点已经变成红色圆点。
2. 点击 VS Code 左侧“运行和调试”。
3. 在顶部选择 `Debug WordFlow`。
4. 按 `F5`。
5. VS Code 会先执行 `build-wordflow` 任务，然后启动 `build/debug/wordflow.exe`。
6. 程序执行到断点时会自动暂停并定位到对应源码。

如果没有出现调试配置，可打开 `.vscode/launch.json`，确认其中存在 `Debug WordFlow`。

## 6. 暂停后的操作

| 操作 | 快捷键 | 作用 |
|---|---|---|
| 继续 | `F5` | 运行到下一个断点 |
| 单步跳过 | `F10` | 执行当前行，不进入被调用函数 |
| 单步进入 | `F11` | 进入当前行调用的函数 |
| 单步跳出 | `Shift+F11` | 运行到当前函数返回 |
| 重启 | `Ctrl+Shift+F5` | 重新启动调试 |
| 停止 | `Shift+F5` | 结束调试进程 |

典型用法：在 `mainwindow.cpp:584` 暂停后按 `F11`，即可进入 `ReviewScheduler::schedule()`。

## 7. 查看变量和调用关系

程序暂停后，查看左侧调试面板：

- **变量**：当前函数参数和局部变量。
- **监视**：持续观察指定表达式。
- **调用堆栈**：显示程序从哪里调用到当前位置。
- **断点**：统一启用、禁用或删除断点。

推荐加入“监视”的表达式：

```cpp
m_currentReviewIndex
m_dueWords.size()
static_cast<int>(rating)
result.intervalDays
result.easeFactor
result.retrievability
wordQuery.numRowsAffected()
```

把鼠标悬停在变量名上，也可以快速查看当前值。

## 8. 各功能的具体调试流程

### 8.1 调试程序启动

1. 在 `main.cpp:7`、`main.cpp:9` 和 `mainwindow.cpp:345` 添加断点。
2. 按 `F5` 启动调试。
3. 在 `main.cpp:7` 按 `F10`，观察 `QApplication` 创建。
4. 到 `main.cpp:9` 后按 `F11`，进入 `MainWindow` 构造函数。
5. 在数据库断点查看两个数据库路径。
6. 按 `F5`，确认窗口正常显示。

### 8.2 调试添加单词

1. 在 `mainwindow.cpp:430` 和 `databasemanager.cpp:162` 添加断点。
2. 按 `F5` 启动程序。
3. 在“单词管理”页填写单词和释义。
4. 点击“添加”。
5. 在 `mainwindow.cpp:430` 检查界面输入。
6. 按 `F11` 进入 `DatabaseManager::addWord()`。
7. 在 `query.exec()` 前检查 SQL 参数。
8. 按 `F10` 执行 SQL；如果返回失败，查看 `query.lastError().text()`。

### 8.3 调试复习算法

1. 确保数据库中至少有一个到期单词。
2. 在 `mainwindow.cpp:584`、`reviewscheduler.cpp:20`、`:38` 和 `databasemanager.cpp:212` 添加断点。
3. 按 `F5`，进入“复习”页。
4. 点击“显示答案”，再选择 Again、Hard、Good 或 Easy。
5. 在 `mainwindow.cpp:584` 查看单词原状态和评分。
6. 按 `F11` 进入调度算法。
7. 使用 `F10` 逐行执行，观察保持率、难度因子和新间隔。
8. 按 `F5` 到数据库事务断点，确认计算结果被保存。

### 8.4 调试词典查询

1. 在 `dictionarymanager.cpp:199` 和 `:204` 添加断点。
2. 启动程序并导入一份 ECDICT CSV。
3. 在 PDF 中选择英文单词并执行自动释义。
4. 在查询入口检查 `word`。
5. 执行查询后检查 `query.next()` 是否成功，以及返回的中文释义。

### 8.5 调试 PDF 选词

1. 在 `pdfselectableview.cpp:171`、`:173` 和 `:274` 添加断点。
2. 启动程序并导入带文本层的 PDF。
3. 用鼠标拖选一个英文单词。
4. 在坐标转换位置检查 `m_dragStart` 和 `endPoint`。
5. 在发出信号前查看 `selectedText()`。
6. 如果文本为空，检查 PDF 是否存在文本层以及页面坐标是否在 `m_pageRect` 内。

## 9. 调试控制台中的 GDB 命令

在 VS Code“调试控制台”中，可以使用 `-exec` 执行 GDB 命令：

```text
-exec bt
-exec info locals
-exec info args
-exec info breakpoints
-exec print wordId
-exec print rating
```

常用含义：

- `bt`：显示调用栈。
- `info locals`：显示当前函数局部变量。
- `info args`：显示函数参数。
- `info breakpoints`：显示全部断点。
- `print`：计算并显示表达式。

## 10. 常见问题

### 10.1 断点是灰色空心圆

可能原因：

- 当前运行的是 Release 版本。
- `wordflow.exe` 不是最新编译结果。
- 断点位于不可执行行。
- 调试器加载的程序不是 `build/debug/wordflow.exe`。

处理方法：

```powershell
qt-cmake --preset debug
cmake --build --preset debug
```

然后确认 `.vscode/launch.json` 中的 `program` 为：

```text
${workspaceFolder}/build/debug/wordflow.exe
```

### 10.2 修改代码后断点位置不正确

重新构建程序并重启调试：

```powershell
cmake --build --preset debug
```

### 10.3 程序停在 Qt 内部源码

查看“调用堆栈”，选择最靠上的项目源码帧，例如：

```text
MainWindow::...
DatabaseManager::...
PdfSelectableView::...
```

通常不需要继续进入 Qt 库内部。

### 10.4 数据库调试结果和预期不一致

当前数据库保存在：

```text
build/debug/wordflow.sqlite3
build/debug/wordflow_dictionary.sqlite3
```

确认调试程序的工作目录是 `build/debug`。不要随意删除该目录，否则会同时删除学习数据和已导入词典。

### 10.5 程序崩溃

程序暂停后首先查看：

1. “调用堆栈”中最上方的项目源码位置。
2. 当前函数参数和局部变量。
3. 指针是否为空。
4. 容器索引是否越界。

在调试控制台执行：

```text
-exec bt
-exec info locals
```

## 11. 推荐的最小断点组合

第一次调试时只设置以下断点即可：

```text
src/main.cpp:7
src/mainwindow.cpp:345
src/mainwindow.cpp:430
src/database/databasemanager.cpp:162
src/mainwindow.cpp:584
src/review/reviewscheduler.cpp:20
src/database/databasemanager.cpp:212
```

这组断点可以完整观察：

```text
程序启动
  → 数据库初始化
  → 添加单词
  → 用户评分
  → 复习算法计算
  → 保存复习记录
```
