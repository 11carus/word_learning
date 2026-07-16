#pragma once

#include <QSqlDatabase>
#include <QDate>
#include <QList>
#include <QString>

struct WordEntry
{
    int id = -1;                 // words 表主键；-1 表示尚未写入数据库。
    QString word;                // 英文单词或词组。
    QString definition;          // 单词释义。
    QString example;             // 可选例句。
    QString source;              // 单词来源，例如 PDF 文件路径。
    QDate nextReviewDate;        // 下次应复习的日期。
    int reviewCount = 0;         // 该单词累计完成的复习次数。
    int intervalDays = 0;        // 当前复习间隔，单位：天；0 表示新单词。
    double easeFactor = 2.3;     // 调度难度因子，越大表示越容易记忆。
    int lapseCount = 0;          // 累计选择 Again 的次数。
    QDate lastReviewDate;        // 最近一次实际复习日期。
    QString createdAt;           // 数据库记录创建时间。
};

class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager();

    bool open(const QString &databasePath);
    QString lastError() const;

    QList<WordEntry> words(const QString &keyword = QString()) const;
    QList<WordEntry> dueWords(const QDate &date = QDate::currentDate()) const;
    bool wordExists(const QString &word) const;
    bool addWord(const QString &word, const QString &definition, const QString &example,
                 const QString &source = QString());
    bool updateWord(int id, const QString &word, const QString &definition, const QString &example);
    bool deleteWord(int id);
    bool recordReview(int wordId, int rating, const QDate &nextReviewDate, int intervalDays,
                      double easeFactor, int lapseCount, const QDate &reviewDate = QDate::currentDate());
    bool resetStudyProgress();
    int totalWordCount() const;
    int totalReviewCount() const;
    int dueWordCount(const QDate &date = QDate::currentDate()) const;
    int reviewedCountOn(const QDate &date = QDate::currentDate()) const;

private:
    bool createTables();
    bool ensureWordColumns();
    void setLastError(const QString &message) const;

    QString m_connectionName;       // Qt SQL 使用的唯一连接名称。
    QSqlDatabase m_database;        // 主词库及复习记录数据库连接。
    mutable QString m_lastError;    // 最近一次数据库错误，供 const 查询函数更新。
};
