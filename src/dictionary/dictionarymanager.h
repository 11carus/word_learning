#pragma once

#include <QString>

class DictionaryManager
{
public:
    DictionaryManager();
    ~DictionaryManager();

    bool open(const QString &databasePath);
    bool importEcdictCsv(const QString &filePath);
    QString lookup(const QString &word) const;
    QString lastError() const;
    int entryCount() const;

private:
    bool createTable();
    void setLastError(const QString &message);

    QString m_connectionName;    // 离线词典数据库的唯一连接名称。
    QString m_lastError;         // 最近一次打开、导入或查询错误。
};
