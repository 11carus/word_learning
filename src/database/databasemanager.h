#pragma once

#include <QSqlDatabase>
#include <QString>

class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager();

    bool open(const QString &databasePath);
    QString lastError() const;

private:
    bool createTables();

    QString m_connectionName;
    QSqlDatabase m_database;
    QString m_lastError;
};

