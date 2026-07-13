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

## 本轮重复复习规则
- 单词初次出现后，选择 Easy 即结束其本轮复习；Again、Hard、Good 会被追加到当前队列末尾。
- 为避免循环，每个单词在一次任务中最多展示 3 次；该阈值只影响当前队列，不改变数据库中的长期复习日期规则。

## 调试辅助功能
- “清空学习进度”会保留 `words` 表中的单词资料，删除 `review_logs`，并把全部单词恢复为当天待复习、复习次数为 0；该操作由 SQLite 事务保证原子性。

## PDF 扩展实现
- 使用 Qt PDF 的 `QPdfDocument::load()` 加载文件，并逐页通过 `getAllText(page).text()` 提取文本；该接口只对带文本层的 PDF 有效。
- PDF 阅读页使用只读文本框展示提取内容；选中符合英文单词格式的文本并填写释义后写入生词本。
- `words.source` 保存来源信息。现有 SQLite 数据库启动时通过 `PRAGMA table_info(words)` 检查字段并执行一次迁移，避免旧数据库无法写入 PDF 生词。

## PDF 阅读与离线自动释义
- Qt PDF Widgets 的 `QPdfView` 可直接绑定 `QPdfDocument` 显示原始 PDF，但它没有公开 Widgets 端的鼠标选词结果，因此不适合作为本项目的选词入口。
- 为实现“直接在左侧 PDF 拖选”，阅读页改用自定义单页渲染控件：`QPdfDocument::render()` 保留原页面版式，鼠标位置按页面点坐标换算后调用 `QPdfDocument::getSelection()`；`QPdfSelection::bounds()` 用于绘制拖选高亮。
- 自定义控件显示单页，并提供上一页、下一页和页码跳转；按阅读区宽度渲染并启用垂直滚动，渲染分辨率跟随屏幕 DPI，避免整页缩放造成模糊。右侧文本层已移除。
- ECDICT 的 CSV 表头包含 `word` 和 `translation` 字段，中文翻译取 `translation`；若该列缺失则回退读取 `definition`。
- 词典导入写入独立的 `wordflow_dictionary.sqlite3`，不与用户的学习数据表混用；导入操作使用事务，导入失败会回滚。
- CSV 解析按 RFC 4180 的引号规则处理逗号、换行和双引号转义，兼容释义中包含换行的词条。完整词库较大，日常使用推荐导入 `ecdict.mini.csv`。

## PDF 缩放与课程交付
- PDF 阅读缩放以“适应宽度”为基准；放大或缩小会改变该基准的倍率，同时启用横向、纵向滚动，以便阅读和精确拖选。
- 高 DPI 页面继续以实际像素渲染，并设置图像设备像素比；缩放只改变阅读比例，不降低渲染清晰度。
- `docs/course-delivery.md` 集中保存报告可用的项目概述、模块关系图、算法说明、测试记录与答辩演示清单；不包含任务书或外部网页的指令性内容。

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
