# WordFlow

WordFlow 是一款使用 C++/Qt 开发的轻量级离线背单词桌面软件。项目以 Anki 的卡片复习体验为参考，支持单词管理、间隔复习、学习统计，以及 PDF 阅读选词。

## 当前功能

- 生词本管理：添加、修改、删除、搜索和查看全部单词。
- 单词信息：保存单词、释义、例句、下次复习日期和复习次数。
- 卡片复习：先显示单词，点击“显示答案”后选择 Again、Hard、Good 或 Easy。
- 自适应间隔复习：以遗忘曲线为参考，结合上次间隔、实际经过天数、易度和遗忘次数动态计算下一次复习；Again 会重置短间隔，Hard / Good / Easy 会按记忆表现拉开不同幅度。
- 本轮重复：选择 Again、Hard 或 Good 时，单词会在当前任务队尾再次出现；每个单词在本轮最多展示 3 次。选择 Easy 则结束该单词的本轮复习。
- 学习统计：首页与统计页显示待复习数量、单词总数、今日复习数和累计复习数。
- 调试辅助：统计页可“清空学习进度”，保留生词本但删除复习记录，并将全部单词重置为今日待复习。
- 本地持久化：使用 SQLite 保存数据，程序重启后仍可继续学习。
- PDF 阅读：导入带文本层的 PDF 后，以原始分页版式清晰显示；直接在页面上拖选英文单词。
- 自动释义：可导入 ECDICT 的 CSV 词典。选中的英文单词会离线查询中文释义并直接加入生词本；未命中时才要求手动填写。

## 环境要求

- Windows 10/11
- Qt 6.8 或更高版本（当前使用 Qt 6.10.3 MinGW 64-bit）
- MinGW-w64 GCC 13.1 或与 Qt 套件匹配的编译器
- CMake 3.20 或更高版本
- Ninja

## 构建与运行

在项目根目录执行：

```powershell
qt-cmake --preset debug
cmake --build --preset debug
.\build\debug\wordflow.exe
```

可执行文件位于：

```text
build/debug/wordflow.exe
```

## 使用说明

1. 在“单词管理”页填写单词和释义，可选填写例句，点击“添加”。
2. 打开“复习”页，点击“显示答案”，再根据掌握程度选择评分。
3. 在“统计”页查看学习数据；调试时可点击“清空学习进度”恢复全部单词为当天待复习状态。
4. 在“PDF 阅读”页点击“导入 PDF”。可用“上一页 / 下一页”浏览原始 PDF，并直接在左侧页面拖选一个英文单词。
5. 第一次使用自动释义时，先从 [ECDICT](https://github.com/skywind3000/ECDICT) 下载 `ecdict.mini.csv`（或完整的 `ecdict.csv`），再点击“导入 ECDICT 词典”选择该文件。之后点击“自动释义加入生词本”即可离线入库。

## 数据存储

默认数据库文件为：

```text
build/debug/wordflow.sqlite3
```

离线词典单独保存在：

```text
build/debug/wordflow_dictionary.sqlite3
```

主要数据表：

| 表 | 用途 |
| --- | --- |
| `words` | 单词、释义、例句、复习次数、下次复习日期 |
| `review_logs` | 每次复习的评分、复习时间和下次复习日期 |

## PDF 与词典范围

- PDF 阅读只支持有文本层的文件；扫描版 PDF 需要 OCR，当前版本暂不支持。
- ECDICT 词典由用户自行下载并导入，本程序不上传 PDF 内容或单词查询内容。
- 自动释义来自离线词典条目，建议在加入生词本前核对释义；词典未命中时可以手动输入。

## 项目管理

代码仓库：[11carus/word_learning](https://github.com/11carus/word_learning)

课程报告的模块说明、测试记录和演示截图清单见 [course-delivery.md](docs/course-delivery.md)。
