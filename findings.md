# WordFlow 发现与决策

## 需求
- 开发轻量级、类似 Anki 的桌面背单词软件。
- 课程 MVP 需要优先完成离线背单词软件：单词管理、卡片复习、四档评价、下次复习日期、SQLite 保存和简单统计。
- 支持导入文本型 PDF，在阅读过程中选择生词并加入生词本；该能力作为核心 MVP 之后的扩展功能。
- 支持生词增删改查、去重、释义、例句和来源记录。
- 支持简化的间隔重复复习和基础学习统计。
- 项目使用 C++/Qt 开发并通过 GitHub 管理。
- 课程项目需体现图形界面、交互操作、算法逻辑和文件读写。

## 课程约束
- 开发语言为 C++，需要程序、课程设计报告和答辩。
- 自选题目要求带图形界面，并包含交互、算法和文件读写。
- `tasks.md` 明确课程项目周期为 4 天，主线技术为 Qt 6 Widgets 和 SQLite。
- `tasks.md` 明确不使用复杂 QML、云同步和完整 FSRS 算法。
- 报告不得直接粘贴源代码；程序清单只描述成员、函数、参数和调用关系。
- 团队应使用 GitHub 或 Gitee 管理，并体现独立完成和组内分工。

## 技术环境
- Qt 6.10.3 MinGW 64位，已安装 QML、Qt Quick、Qt PDF 和 Qt PDF Quick。
- Qt Widgets 和 Qt SQL 属于当前 Qt 6 安装可用的课程 MVP 目标模块，后续项目骨架阶段需要通过 CMake 再做一次链接验证。
- Qt兼容工具链为 MinGW-w64 GCC/G++ 13.1.0、GDB 11.2。
- 构建工具为 CMake 4.3.1 和 Ninja 1.13.2。
- Qt Creator 20.0.0 已安装，未安装遥测插件。
- 最小 C++20 探测程序已成功链接 Core、QML、Quick、PDF 和 PDF Quick。
- 可选 Vulkan 头文件未安装，不影响 Windows 默认 Direct3D 渲染路径。
- Qt Widgets 和 Qt SQL 已通过 `qt-cmake --preset debug` 与 Ninja 实际编译验证。
- 运行 `wordflow.exe` 后成功创建本地 SQLite 文件 `build/debug/wordflow.sqlite3`。
- GitHub 远程仓库已确定为 `https://github.com/11carus/word_learning.git`。

## 技术决策
| 决策 | 理由 |
|------|------|
| 课程 MVP 使用 Qt Widgets | 与 `tasks.md` 明确要求一致，4 天内更稳 |
| 数据存储使用 SQLite | 与 `tasks.md` 一致，满足离线和重启后数据保留 |
| PDF 第一版仅处理有文本层的文件 | OCR 会显著增加依赖、代码量和测试风险 |
| PDF 功能作为扩展 | 保留阅读选词特色，同时不影响课程 MVP |
| 复习调度先实现固定间隔序列：Again 1天、Hard 2天、Good 4天、Easy 7天 | 逻辑清晰、便于测试，适合课程设计范围 |
| Git 工作流采用 main＋feature 分支 | 保持主分支可运行，并体现成员贡献 |
| 当前源码骨架采用单目标 `wordflow` | 项目还处于 MVP 阶段，避免过早拆分复杂模块 |

## 当前数据库表
| 表 | 作用 |
|------|------|
| `words` | 保存单词、释义、例句、下次复习日期、复习次数和创建时间 |
| `review_logs` | 保存每次复习的单词、评价、复习时间和下次复习日期 |

## 复习实现约定
- 今日任务查询条件为 `next_review_date <= 当天`，因此逾期单词会继续出现在待复习列表中。
- 每次评分用 SQLite 事务写入一条 `review_logs`，并同步更新 `words.next_review_date` 与 `words.review_count`。
- 当前统计为基础累计统计；完整图表和按日期趋势统计可在后续扩展。

## 待确认
- 是否加入本地词典，或者第一版只允许手动输入释义。
- 团队成员分工。
- 最终产品名使用 WordFlow、Read2Remember 或其他名称。
- PDF 扩展是否必须在最终演示中出现，还是作为加分功能说明。

## 资源
- 课程任务书：`程序语言课程设计-2026.pptx`
- 技能安装记录：`docs/installed-skills.md`
- 开发环境记录：`docs/development-environment.md`

## 视觉/浏览器发现
- Qt 官方资料确认 Qt 6.10 Windows MinGW配置使用 MinGW-w64 13.1。
- Qt下载仓库确认 Qt 6.10.3 提供 `win64_mingw` 和 `qtpdf` 模块。

---
*新的研究结论、外部资料和技术决策持续追加到本文件。*
