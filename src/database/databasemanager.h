#pragma once

#include <QSqlDatabase>
#include <QDate>
#include <QList>
#include <QString>

struct WordEntry
{
    int id = -1;
    QString word;
    QString definition;
    QString example;
    QDate nextReviewDate;
    int reviewCount = 0;
    QString createdAt;
};

class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager();

    bool open(const QString &databasePath);
    QString lastError() const;

    QList<WordEntry> words(const QString &keyword = QString()) const;
    bool addWord(const QString &word, const QString &definition, const QString &example);
    bool updateWord(int id, const QString &word, const QString &definition, const QString &example);
    bool deleteWord(int id);
    int totalWordCount() const;

private:
    bool createTables();
    void setLastError(const QString &message) const;

    QString m_connectionName;
    QSqlDatabase m_database;
    mutable QString m_lastError;
};
