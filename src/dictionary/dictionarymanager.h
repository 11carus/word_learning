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

    QString m_connectionName;
    QString m_lastError;
};
