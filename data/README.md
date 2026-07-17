# 本地数据

此目录用于保存不提交到 Git 的本地输入数据。

- `ecdict.csv`：供程序手动导入的 ECDICT 离线词典，已由 `.gitignore` 忽略。

程序运行时生成的学习数据库不放在这里，而是与可执行文件位于同一目录：

- `build/debug/wordflow.sqlite3`：生词与复习记录。
- `build/debug/wordflow_dictionary.sqlite3`：导入后生成的本地词典数据库。

删除 `build/` 会同时删除其中的运行数据库；如需保留学习记录，请先备份对应的 `.sqlite3` 文件。
