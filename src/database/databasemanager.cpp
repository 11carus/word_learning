#include "database/databasemanager.h"

#include <QDate>
#include <QSqlError>
#include <QSqlQuery>
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
        SELECT id, word, definition, example, source, next_review_date, review_count, created_at
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
        entry.createdAt = query.value(7).toString();
        entries.append(entry);
    }

    return entries;
}

QList<WordEntry> DatabaseManager::dueWords(const QDate &date) const
{
    QList<WordEntry> entries;
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(R"(
        SELECT id, word, definition, example, source, next_review_date, review_count, created_at
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
        entry.createdAt = query.value(7).toString();
        entries.append(entry);
    }

    return entries;
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

bool DatabaseManager::recordReview(int wordId, int rating, const QDate &nextReviewDate)
{
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
            review_count = review_count + 1
        WHERE id = :id
    )"));
    wordQuery.bindValue(QStringLiteral(":next_review_date"), nextReviewDate.toString(Qt::ISODate));
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
            review_count = 0
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

    return ensureWordSourceColumn();
}

bool DatabaseManager::ensureWordSourceColumn()
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("PRAGMA table_info(words)"))) {
        m_lastError = query.lastError().text();
        return false;
    }

    while (query.next()) {
        if (query.value(1).toString() == QStringLiteral("source")) {
            return true;
        }
    }

    if (!query.exec(QStringLiteral("ALTER TABLE words ADD COLUMN source TEXT"))) {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

void DatabaseManager::setLastError(const QString &message) const
{
    m_lastError = message;
}
