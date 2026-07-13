# WordFlow 实时进度与改动日志

> 更新约定：每次完成实现、配置、测试、文档修改或重要决策后立即更新。记录采用时间顺序追加，不覆盖历史。

## 当前状态
- **当前阶段：** 阶段 5——PDF 阅读选词扩展
- **总体状态：** 课程 MVP 已通过用户手动验收，README 已完成更新
- **下一步：** 实现文本型 PDF 导入、阅读与选词加入生词本
- **最后更新：** 2026-07-13 15:40:00 +08:00

## 2026-07-13

### 1. 课程任务书读取
- **状态：** complete
- 完整读取 `程序语言课程设计-2026.pptx` 的24页内容。
- 提取课程时间、提交节点、评分方式、报告结构和自选题目要求。
- 确认项目需使用 C++、包含图形界面/交互/算法/文件读写，并通过 GitHub 或 Gitee 管理。
- **改动文件：** 无。

### 2. 项目方向确定
- **状态：** complete
- 确定开发轻量级 Anki 式背单词软件。
- 核心特色为导入文本型 PDF、阅读选词并加入生词本。
- 初步范围包括生词本、简化间隔重复、PDF阅读选词和学习统计。
- **改动文件：** 无。

### 3. Codex Skills 安装
- **状态：** complete
- 项目级安装7个 Qt技能和7个 C++工程技能。
- 全局安装 `planning-with-files-zh`。
- 审查 README、SKILL frontmatter 和脚本，完成逐文件一致性验证。
- **创建/修改：**
  - `.agents/skills/`
  - `docs/installed-skills.md`

### 4. Qt/C++ 开发环境安装
- **状态：** complete
- 安装 Qt 6.10.3、QML/Qt Quick、Qt PDF/Qt PDF Quick。
- 安装 MinGW 13.1.0、GDB 11.2、Qt Creator 20.0.0和Ninja 1.13.2。
- 更新用户 PATH，使新工具链优先于原有 MinGW 8.1。
- 未安装 Qt Creator 遥测插件、无关模块、源码和调试符号。
- **创建/修改：**
  - `D:\Qt\6.10.3\mingw_64`
  - `D:\Qt\Tools\mingw1310_64`
  - `D:\Qt\Tools\QtCreator`
  - `docs/development-environment.md`
  - `docs/installed-skills.md`

### 5. 规划与实时进度机制
- **状态：** complete
- 建立根目录规划、发现和实时进度文件。
- 将此前已经完成的任务、环境状态、错误和技术决策补录到磁盘。
- 约定之后每次实现、测试和重要改动同步更新本日志。
- **创建/修改：**
  - `task_plan.md`
  - `findings.md`
  - `progress.md`

### 6. 合并 tasks.md
- **状态：** complete
- 重新读取项目根目录的 `tasks.md`，文件大小为 14456 字节。
- 任务书要求 4 天内完成 Qt 6 Widgets + SQLite 的离线背单词课程项目。
- 已将原计划从“QML/PDF 优先”调整为“Qt Widgets/SQLite 课程 MVP 优先，PDF 阅读选词作为扩展功能”。
- 已补充四天开发安排、T01-T10 验收用例、SQLite 表结构方向和核心模块方向。
- 项目根目录目前还不是 Git 仓库，因此无法使用 `git diff` 辅助核对改动；已记录到后续阶段 3。
- 已用关键字核对 `task_plan.md`、`findings.md`、`progress.md`，确认主线已切换为 Qt Widgets/SQLite，PDF 阅读选词保留为扩展阶段。
- 运行 `planning-with-files-zh` 的 `session-catchup.py` 后提示刚刚的核对动作未同步，已补写到本日志。
- **改动文件：**
  - `task_plan.md`（记录本次问题）
  - `findings.md`（记录任务书约束和技术取舍）
  - `progress.md`（记录合并状态）

### 7. 仓库与 Qt Widgets 项目骨架
- **状态：** complete
- 初始化本地 Git 仓库，默认分支为 `main`。
- 创建 Qt 6 Widgets/CMake 项目骨架，目标名为 `wordflow`。
- 配置 `CMakePresets.json`，包含 Debug 和 Release 两个 Ninja preset。
- 创建 `.gitignore`，排除构建目录、Qt Creator 用户文件、Codex 本地技能目录、安装日志、Office 临时文件和 SQLite 运行数据。
- 创建最小主窗口，包含首页、单词管理、复习、统计四个标签页。
- 创建 `DatabaseManager`，启动时初始化 SQLite 数据库和 `words`、`review_logs` 两张表。
- 创建 `ReviewScheduler`，实现 Again/Hard/Good/Easy 对应 1/2/4/7 天的日期计算。
- 使用 `qt-cmake --preset debug` 配置成功，使用 `cmake --build --preset debug` 编译成功。
- 短启动 `wordflow.exe` 后程序保持运行，未在 3 秒内崩溃，并成功创建 `build/debug/wordflow.sqlite3`。
- 使用 `sqlite3` 验证数据库中存在 `words` 和 `review_logs` 表。
- 最终 `git status --short` 显示项目文件均为未跟踪新文件，尚未创建提交；`git diff --stat` 为空属于新仓库未跟踪文件的正常表现。
- 用户提供 GitHub 仓库 `11carus/word_learning` 后，已添加远程 `origin`。
- 远程 `main` 已有一个初始 README 提交，本地分支已切换为跟踪 `origin/main`，避免覆盖远程历史。
- 本机未配置 Git 提交身份，已在仓库本地配置 `11carus <11carus@users.noreply.github.com>`。
- 创建提交 `b743fee Initialize Qt Widgets project skeleton`。
- 已推送到 `origin/main`，远程从 `b2109d2` 更新到 `b743fee`。
- **创建/修改：**
  - `.gitignore`
  - `CMakeLists.txt`
  - `CMakePresets.json`
  - `src/main.cpp`
  - `src/mainwindow.h`
  - `src/mainwindow.cpp`
  - `src/database/databasemanager.h`
  - `src/database/databasemanager.cpp`
  - `src/review/reviewscheduler.h`
  - `src/review/reviewscheduler.cpp`
  - `README.md`
  - `task_plan.md`
  - `findings.md`
  - `progress.md`

### 8. 单词管理 CRUD
- **状态：** complete
- 扩展 `DatabaseManager`，新增 `WordEntry` 数据结构和 `words()`、`addWord()`、`updateWord()`、`deleteWord()`、`totalWordCount()` 方法。
- 数据库写入统一使用参数化 SQL，避免手动拼接用户输入。
- 添加启动时 `PRAGMA foreign_keys = ON`，为后续删除单词时同步删除复习记录做准备。
- 将单词管理页从占位文本改为可用界面：表单输入单词、释义、例句，表格显示生词本。
- 实现添加、修改、删除、搜索、查看全部和清空表单。
- 删除单词前会弹出确认提示，并说明相关复习记录会一起删除。
- 首页单词总数会在添加、修改、删除后刷新。
- 使用 `cmake --build --preset debug` 编译成功。
- 短启动 `wordflow.exe` 后程序保持运行，未在 3 秒内崩溃。
- 使用 `sqlite3` 确认 `wordflow.sqlite3` 中仍存在 `words` 和 `review_logs` 表。
- **修改文件：**
  - `src/database/databasemanager.h`
  - `src/database/databasemanager.cpp`
  - `src/mainwindow.h`
  - `src/mainwindow.cpp`
  - `task_plan.md`
  - `progress.md`

### 9. 卡片复习与简单统计
- **状态：** complete
- 新增今日到期单词查询：按下次复习日期排序显示复习卡片。
- 复习页遵循“先显示单词、点击显示答案、再选择 Again/Hard/Good/Easy”流程；四档评分仍按 1/2/4/7 天安排下次复习。
- `recordReview()` 使用 SQLite 事务同时插入 `review_logs` 并更新单词的下次复习日期、复习次数。
- 首页实时显示今日待复习、单词总数和今日已复习；统计页显示单词总数、累计复习、今日已复习和当前待复习。
- 使用 `cmake --build --preset debug` 编译成功。
- 短启动 `wordflow.exe` 后程序保持运行，未在 3 秒内崩溃；用 `sqlite3` 核验了 `review_logs` 表结构。
- **修改文件：**
  - `src/database/databasemanager.h`
  - `src/database/databasemanager.cpp`
  - `src/mainwindow.h`
  - `src/mainwindow.cpp`
  - `task_plan.md`
  - `progress.md`

### 10. 本轮重复复习阈值
- **状态：** complete
- 新增本轮学习队列规则：Easy 不再入队；Again、Hard、Good 会加入队尾再次复习。
- 每个单词在一次任务中最多展示 3 次，避免难词无限循环；状态栏会提示当前重复次数或已达到上限。
- 保留原有的 1/2/4/7 天下次复习日期规则，队列重排仅影响本轮学习体验。
- **修改文件：**
  - `src/mainwindow.h`
  - `src/mainwindow.cpp`
  - `task_plan.md`
  - `progress.md`

### 11. 调试刷新修复与构建验证
- **状态：** complete
- 验收中发现：新增单词后直接切换到复习页不会刷新待复习队列；已改为进入复习页时自动刷新。
- 先前中断遗留的 `cmake/ninja` 构建进程已停止，随后重新构建成功。
- **修改文件：**
  - `src/mainwindow.cpp`
  - `task_plan.md`
  - `progress.md`

### 12. 清空学习进度
- **状态：** complete
- 在统计页新增“清空学习进度”按钮及二次确认提示。
- 清空操作保留单词、释义和例句；通过 SQLite 事务删除全部复习记录，并将所有单词的复习次数设为 0、下次复习日期设为当天。
- 使用 `cmake --build --preset debug` 编译成功。
- **修改文件：**
  - `src/database/databasemanager.h`
  - `src/database/databasemanager.cpp`
  - `src/mainwindow.h`
  - `src/mainwindow.cpp`
  - `task_plan.md`
  - `progress.md`

### 13. README 优先整理
- **状态：** complete
- 用户确认已手动验证核心功能；阶段 6 的验收项据此标记完成。
- 更新 README：功能清单、构建运行步骤、使用说明、数据位置、数据库表说明和 PDF 扩展范围。
- **修改文件：**
  - `README.md`
  - `task_plan.md`
  - `progress.md`

## 测试结果
| 测试 | 输入 | 预期结果 | 实际结果 | 状态 |
|------|------|---------|---------|------|
| Skill完整性 | 15个目标Skill | 均包含SKILL.md且名称匹配 | 15/15通过 | passed |
| Qt版本 | `qmake -v` / `qtpaths` | Qt 6.x可用 | Qt 6.10.3 | passed |
| 编译器版本 | Qt工具链 `g++ --version` | 与Qt二进制匹配 | GCC 13.1.0 | passed |
| Ninja版本 | `ninja --version` | Ninja可执行 | 1.13.2 | passed |
| QML工具 | `qmllint --version` | QML工具可执行 | 6.10.3 | passed |
| Qt PDF组件 | CMake查找Pdf/PdfQuick | 两个组件均可用 | 成功找到并链接 | passed |
| C++20构建探测 | CMake＋Ninja最小程序 | 编译、链接、运行成功 | 退出码0 | passed |
| Qt Widgets/SQL配置 | `qt-cmake --preset debug` | 生成 Ninja 构建文件 | 配置成功 | passed |
| 项目编译 | `cmake --build --preset debug` | 生成 `wordflow.exe` | 编译链接成功 | passed |
| 短启动检查 | 启动 `wordflow.exe` 3 秒 | 程序不崩溃 | 进程保持运行，随后手动关闭 | passed |
| SQLite初始化 | 查询 `wordflow.sqlite3` | 存在 `words` 和 `review_logs` | 两张表均存在 | passed |
| 单词管理编译 | `cmake --build --preset debug` | CRUD代码编译通过 | 编译链接成功 | passed |
| 单词管理短启动 | 启动 `wordflow.exe` 3 秒 | 程序不崩溃 | 进程保持运行，随后手动关闭 | passed |
| 复习与统计编译 | `cmake --build --preset debug` | 复习和统计代码编译通过 | 编译链接成功 | passed |
| 复习与统计短启动 | 启动 `wordflow.exe` 3 秒 | 程序不崩溃 | 进程保持运行，随后手动关闭 | passed |
| 本轮重复阈值编译 | `cmake --build --preset debug` | 队列重排与阈值代码编译通过 | 编译链接成功 | passed |
| 清空学习进度编译 | `cmake --build --preset debug` | 重置数据库与统计页按钮代码编译通过 | 编译链接成功 | passed |

## 错误日志
| 环节 | 错误 | 尝试次数 | 解决方案 | 状态 |
|------|------|---------|---------|------|
| 任务书读取 | PPT渲染依赖路径异常 | 1 | 直接解析PPTX XML | resolved |
| 附件读取 | 默认编码导致中文乱码 | 1 | 使用UTF-8重新读取 | resolved |
| 技能审查 | Windows路径转义导致路径失真 | 2 | 改用正斜杠和安全路径表达 | resolved |
| aqt安装 | pip命令到达时间上限 | 1 | 检查确认软件实际已安装 | resolved |
| Qt查询 | 镜像校验文件临时下载失败 | 1 | 重试后成功 | resolved |
| 工具链验证 | MinGW目录名预期错误 | 1 | 搜索真实目录并更新路径 | resolved |
| 任务合并 | `tasks.md` 初次为0字节 | 1 | 用户重新指定文件后读取到内容并完成合并 | resolved |
| Git核对 | 项目尚未初始化Git仓库 | 1 | 已执行 `git init -b main` | resolved |
| GitHub远程 | 远程仓库地址尚未提供 | 1 | 用户提供仓库后已添加 `origin` 并抓取远程 `main` | resolved |
| Git提交身份 | 本机未配置 `user.name` / `user.email` | 1 | 已在仓库本地配置 GitHub noreply 身份 | resolved |

## 五问重启检查
| 问题 | 答案 |
|------|------|
| 我在哪里？ | 阶段4：课程 MVP 核心功能实现，单词管理 CRUD 已完成 |
| 我要去哪里？ | 完成设计、仓库骨架、核心功能、测试和课程交付 |
| 目标是什么？ | 完成支持PDF阅读选词与间隔复习的Qt背单词软件 |
| 我学到了什么？ | 见 `findings.md` |
| 我做了什么？ | 已完成需求梳理、技能安装、环境安装、任务书合并、Qt Widgets/CMake/SQLite 骨架和 GitHub 首次推送 |

---
*每个阶段完成后、每次测试后或出现错误时更新本文件。*
