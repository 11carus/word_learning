#include "database/databasemanager.h"

#include <QDate>
#include <QSqlError>
#include <QSqlQuery>
#include <QSet>
#include <QVariant>

DatabaseManager::DatabaseManager()
    : m_connectionName(QStringLiteral("wordflow-main"))
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_database.isValid()) {
        m_database.close();
    }
}

bool DatabaseManager::open(const QString &databasePath)
{
    if (QSqlDatabase::contains(m_connectionName)) {
        m_database = QSqlDatabase::database(m_connectionName);
    } else {
        m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    }

    m_database.setDatabaseName(databasePath);

    if (!m_database.open()) {
        m_lastError = m_database.lastError().text();
        return false;
    }

    QSqlQuery pragmaQuery(m_database);
    pragmaQuery.exec(QStringLiteral("PRAGMA foreign_keys = ON"));

    return createTables();
}

QString DatabaseManager::lastError() const
{
    return m_lastError;
}

QList<WordEntry> DatabaseManager::words(const QString &keyword) const
{
    QList<WordEntry> entries;

    QString sql = QStringLiteral(R"(
        SELECT id, word, definition, example, source, next_review_date, review_count,
               interval_days, ease_factor, lapse_count, last_review_date, created_at
        FROM words
    )");

    if (!keyword.trimmed().isEmpty()) {
        sql += QStringLiteral(" WHERE word LIKE :keyword OR definition LIKE :keyword");
    }

    sql += QStringLiteral(" ORDER BY word COLLATE NOCASE ASC");

    QSqlQuery query(m_database);
    if (!query.prepare(sql)) {
        setLastError(query.lastError().text());
        return entries;
    }

    if (!keyword.trimmed().isEmpty()) {
        query.bindValue(QStringLiteral(":keyword"), QStringLiteral("%%1%").arg(keyword.trimmed()));
    }

    if (!query.exec()) {
        setLastError(query.lastError().text());
        return entries;
    }

    while (query.next()) {
        WordEntry entry;
        entry.id = query.value(0).toInt();
        entry.word = query.value(1).toString();
        entry.definition = query.value(2).toString();
        entry.example = query.value(3).toString();
        entry.source = query.value(4).toString();
        entry.nextReviewDate = QDate::fromString(query.value(5).toString(), Qt::ISODate);
        entry.reviewCount = query.value(6).toInt();
        entry.intervalDays = query.value(7).toInt();
        entry.easeFactor = query.value(8).toDouble();
        entry.lapseCount = query.value(9).toInt();
        entry.lastReviewDate = QDate::fromString(query.value(10).toString(), Qt::ISODate);
        entry.createdAt = query.value(11).toString();
        entries.append(entry);
    }

    return entries;
}

QList<WordEntry> DatabaseManager::dueWords(const QDate &date) const
{
    QList<WordEntry> entries;
    QSqlQuery query(m_database);

    // 每日复习队列只包含今天及以前到期的单词。
    // 先按到期日期升序排列，让逾期最久的单词优先；日期相同时，
    // 再按忽略大小写的字母顺序排列，保证展示顺序稳定。
    query.prepare(QStringLiteral(R"(
        SELECT id, word, definition, example, source, next_review_date, review_count,
               interval_days, ease_factor, lapse_count, last_review_date, created_at
        FROM words
        WHERE next_review_date <= :date
        ORDER BY next_review_date ASC, word COLLATE NOCASE ASC
    )"));
    query.bindValue(QStringLiteral(":date"), date.toString(Qt::ISODate));

    if (!query.exec()) {
        setLastError(query.lastError().text());
        return entries;
    }

    while (query.next()) {
        WordEntry entry;
        entry.id = query.value(0).toInt();
        entry.word = query.value(1).toString();
        entry.definition = query.value(2).toString();
        entry.example = query.value(3).toString();
        entry.source = query.value(4).toString();
        entry.nextReviewDate = QDate::fromString(query.value(5).toString(), Qt::ISODate);
        entry.reviewCount = query.value(6).toInt();
        entry.intervalDays = query.value(7).toInt();
        entry.easeFactor = query.value(8).toDouble();
        entry.lapseCount = query.value(9).toInt();
        entry.lastReviewDate = QDate::fromString(query.value(10).toString(), Qt::ISODate);
        entry.createdAt = query.value(11).toString();
        entries.append(entry);
    }

    return entries;
}

bool DatabaseManager::wordExists(const QString &word) const
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("SELECT 1 FROM words WHERE word = :word COLLATE NOCASE LIMIT 1"));
    query.bindValue(QStringLiteral(":word"), word.trimmed());
    return query.exec() && query.next();
}

bool DatabaseManager::addWord(const QString &word, const QString &definition, const QString &example,
                              const QString &source)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(R"(
        INSERT INTO words (word, definition, example, source, next_review_date)
        VALUES (:word, :definition, :example, :source, :next_review_date)
    )"));
    query.bindValue(QStringLiteral(":word"), word.trimmed());
    query.bindValue(QStringLiteral(":definition"), definition.trimmed());
    query.bindValue(QStringLiteral(":example"), example.trimmed());
    query.bindValue(QStringLiteral(":source"), source.trimmed());
    query.bindValue(QStringLiteral(":next_review_date"), QDate::currentDate().toString(Qt::ISODate));

    if (!query.exec()) {
        setLastError(query.lastError().text());
        return false;
    }

    return true;
}

bool DatabaseManager::updateWord(int id, const QString &word, const QString &definition, const QString &example)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(R"(
        UPDATE words
        SET word = :word,
            definition = :definition,
            example = :example
        WHERE id = :id
    )"));
    query.bindValue(QStringLiteral(":word"), word.trimmed());
    query.bindValue(QStringLiteral(":definition"), definition.trimmed());
    query.bindValue(QStringLiteral(":example"), example.trimmed());
    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec()) {
        setLastError(query.lastError().text());
        return false;
    }

    return true;
}

bool DatabaseManager::deleteWord(int id)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM words WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec()) {
        setLastError(query.lastError().text());
        return false;
    }

    return true;
}

bool DatabaseManager::recordReview(int wordId, int rating, const QDate &nextReviewDate, int intervalDays,
                                   double easeFactor, int lapseCount, const QDate &reviewDate)
{
    // 写入复习日志和更新单词调度状态必须同时成功。
    // 使用事务可以避免只写入其中一张表而造成数据不一致。
    if (!m_database.transaction()) {
        setLastError(m_database.lastError().text());
        return false;
    }

    QSqlQuery logQuery(m_database);
    logQuery.prepare(QStringLiteral(R"(
        INSERT INTO review_logs (word_id, rating, next_review_date)
        VALUES (:word_id, :rating, :next_review_date)
    )"));
    logQuery.bindValue(QStringLiteral(":word_id"), wordId);
    logQuery.bindValue(QStringLiteral(":rating"), rating);
    logQuery.bindValue(QStringLiteral(":next_review_date"), nextReviewDate.toString(Qt::ISODate));

    if (!logQuery.exec()) {
        setLastError(logQuery.lastError().text());
        m_database.rollback();
        return false;
    }

    QSqlQuery wordQuery(m_database);
    wordQuery.prepare(QStringLiteral(R"(
        UPDATE words
        SET next_review_date = :next_review_date,
            review_count = review_count + 1,
            interval_days = :interval_days,
            ease_factor = :ease_factor,
            lapse_count = :lapse_count,
            last_review_date = :last_review_date
        WHERE id = :id
    )"));
    wordQuery.bindValue(QStringLiteral(":next_review_date"), nextReviewDate.toString(Qt::ISODate));
    wordQuery.bindValue(QStringLiteral(":interval_days"), intervalDays);
    wordQuery.bindValue(QStringLiteral(":ease_factor"), easeFactor);
    wordQuery.bindValue(QStringLiteral(":lapse_count"), lapseCount);
    wordQuery.bindValue(QStringLiteral(":last_review_date"), reviewDate.toString(Qt::ISODate));
    wordQuery.bindValue(QStringLiteral(":id"), wordId);

    if (!wordQuery.exec() || wordQuery.numRowsAffected() != 1) {
        setLastError(wordQuery.lastError().text());
        m_database.rollback();
        return false;
    }

    if (!m_database.commit()) {
        setLastError(m_database.lastError().text());
        m_database.rollback();
        return false;
    }

    return true;
}

bool DatabaseManager::resetStudyProgress()
{
    if (!m_database.transaction()) {
        setLastError(m_database.lastError().text());
        return false;
    }

    QSqlQuery clearLogsQuery(m_database);
    if (!clearLogsQuery.exec(QStringLiteral("DELETE FROM review_logs"))) {
        setLastError(clearLogsQuery.lastError().text());
        m_database.rollback();
        return false;
    }

    QSqlQuery resetWordsQuery(m_database);
    resetWordsQuery.prepare(QStringLiteral(R"(
        UPDATE words
        SET next_review_date = :today,
            review_count = 0,
            interval_days = 0,
            ease_factor = 2.3,
            lapse_count = 0,
            last_review_date = NULL
    )"));
    resetWordsQuery.bindValue(QStringLiteral(":today"), QDate::currentDate().toString(Qt::ISODate));

    if (!resetWordsQuery.exec()) {
        setLastError(resetWordsQuery.lastError().text());
        m_database.rollback();
        return false;
    }

    if (!m_database.commit()) {
        setLastError(m_database.lastError().text());
        m_database.rollback();
        return false;
    }

    return true;
}

int DatabaseManager::totalWordCount() const
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM words"))) {
        setLastError(query.lastError().text());
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int DatabaseManager::totalReviewCount() const
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM review_logs"))) {
        setLastError(query.lastError().text());
        return 0;
    }

    return query.next() ? query.value(0).toInt() : 0;
}

int DatabaseManager::dueWordCount(const QDate &date) const
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM words WHERE next_review_date <= :date"));
    query.bindValue(QStringLiteral(":date"), date.toString(Qt::ISODate));

    if (!query.exec()) {
        setLastError(query.lastError().text());
        return 0;
    }

    return query.next() ? query.value(0).toInt() : 0;
}

int DatabaseManager::reviewedCountOn(const QDate &date) const
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(R"(
        SELECT COUNT(*) FROM review_logs
        WHERE DATE(review_date, 'localtime') = :date
    )"));
    query.bindValue(QStringLiteral(":date"), date.toString(Qt::ISODate));

    if (!query.exec()) {
        setLastError(query.lastError().text());
        return 0;
    }

    return query.next() ? query.value(0).toInt() : 0;
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);

    if (!query.exec(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS words (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            word TEXT NOT NULL UNIQUE,
            definition TEXT NOT NULL,
            example TEXT,
            source TEXT,
            next_review_date TEXT NOT NULL,
            review_count INTEGER NOT NULL DEFAULT 0,
            interval_days INTEGER NOT NULL DEFAULT 0,
            ease_factor REAL NOT NULL DEFAULT 2.3,
            lapse_count INTEGER NOT NULL DEFAULT 0,
            last_review_date TEXT,
            created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
        )
    )"))) {
        m_lastError = query.lastError().text();
        return false;
    }

    if (!query.exec(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS review_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            word_id INTEGER NOT NULL,
            rating INTEGER NOT NULL,
            review_date TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
            next_review_date TEXT NOT NULL,
            FOREIGN KEY(word_id) REFERENCES words(id) ON DELETE CASCADE
        )
    )"))) {
        m_lastError = query.lastError().text();
        return false;
    }

    return ensureWordColumns();
}

bool DatabaseManager::ensureWordColumns()
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("PRAGMA table_info(words)"))) {
        m_lastError = query.lastError().text();
        return false;
    }

    QSet<QString> columns;
    while (query.next()) {
        columns.insert(query.value(1).toString());
    }

    const QList<QPair<QString, QString>> missingColumns = {
        {QStringLiteral("source"), QStringLiteral("TEXT")},
        {QStringLiteral("interval_days"), QStringLiteral("INTEGER NOT NULL DEFAULT 0")},
        {QStringLiteral("ease_factor"), QStringLiteral("REAL NOT NULL DEFAULT 2.3")},
        {QStringLiteral("lapse_count"), QStringLiteral("INTEGER NOT NULL DEFAULT 0")},
        {QStringLiteral("last_review_date"), QStringLiteral("TEXT")},
    };

    for (const auto &[name, definition] : missingColumns) {
        if (columns.contains(name)) {
            continue;
        }
        if (!query.exec(QStringLiteral("ALTER TABLE words ADD COLUMN %1 %2").arg(name, definition))) {
            m_lastError = query.lastError().text();
            return false;
        }
    }

    return true;
}

void DatabaseManager::setLastError(const QString &message) const
{
    m_lastError = message;
}
