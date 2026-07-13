#include "dictionarymanager.h"

#include <QFile>
#include <QHash>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QUuid>

namespace {
QString normalizedHeader(const QString &value)
{
    QString header = value.trimmed().toLower();
    if (!header.isEmpty() && header.front() == QChar::ByteOrderMark) {
        header.removeFirst();
    }
    return header;
}
}

DictionaryManager::DictionaryManager()
    : m_connectionName(QStringLiteral("wordflow-dictionary-") + QUuid::createUuid().toString(QUuid::WithoutBraces))
{
}

DictionaryManager::~DictionaryManager()
{
    if (QSqlDatabase::contains(m_connectionName)) {
        {
            QSqlDatabase database = QSqlDatabase::database(m_connectionName);
            database.close();
        }
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool DictionaryManager::open(const QString &databasePath)
{
    QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    database.setDatabaseName(databasePath);
    if (!database.open()) {
        setLastError(database.lastError().text());
        return false;
    }

    return createTable();
}

bool DictionaryManager::createTable()
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS dictionary_entries (
            word TEXT PRIMARY KEY COLLATE NOCASE,
            translation TEXT NOT NULL
        )
    )"))) {
        setLastError(query.lastError().text());
        return false;
    }

    return true;
}

bool DictionaryManager::importEcdictCsv(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        setLastError(file.errorString());
        return false;
    }

    const QString csv = QString::fromUtf8(file.readAll());
    if (csv.isEmpty()) {
        setLastError(QObject::tr("词典文件为空或不是 UTF-8 编码。"));
        return false;
    }

    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    if (!database.transaction()) {
        setLastError(database.lastError().text());
        return false;
    }

    QSqlQuery clearQuery(database);
    if (!clearQuery.exec(QStringLiteral("DELETE FROM dictionary_entries"))) {
        database.rollback();
        setLastError(clearQuery.lastError().text());
        return false;
    }

    QSqlQuery insertQuery(database);
    insertQuery.prepare(QStringLiteral(
        "INSERT OR REPLACE INTO dictionary_entries (word, translation) VALUES (:word, :translation)"));

    QString field;
    QStringList record;
    QHash<QString, int> columns;
    bool inQuotes = false;
    bool headerRead = false;
    int importedCount = 0;
    bool failed = false;

    const auto consumeRecord = [&]() {
        record.append(field);
        field.clear();

        if (!headerRead) {
            for (int index = 0; index < record.size(); ++index) {
                columns.insert(normalizedHeader(record.at(index)), index);
            }
            headerRead = true;
        } else {
            const int wordColumn = columns.value(QStringLiteral("word"), -1);
            int translationColumn = columns.value(QStringLiteral("translation"), -1);
            if (translationColumn < 0) {
                translationColumn = columns.value(QStringLiteral("definition"), -1);
            }
            if (wordColumn >= 0 && translationColumn >= 0
                && wordColumn < record.size() && translationColumn < record.size()) {
                const QString word = record.at(wordColumn).trimmed();
                const QString translation = record.at(translationColumn).trimmed();
                if (!word.isEmpty() && !translation.isEmpty()) {
                    insertQuery.bindValue(QStringLiteral(":word"), word);
                    insertQuery.bindValue(QStringLiteral(":translation"), translation);
                    if (!insertQuery.exec()) {
                        setLastError(insertQuery.lastError().text());
                        failed = true;
                    } else {
                        ++importedCount;
                    }
                }
            }
        }
        record.clear();
    };

    for (qsizetype index = 0; index < csv.size() && !failed; ++index) {
        const QChar character = csv.at(index);
        if (inQuotes) {
            if (character == QLatin1Char('"')) {
                if (index + 1 < csv.size() && csv.at(index + 1) == QLatin1Char('"')) {
                    field.append(character);
                    ++index;
                } else {
                    inQuotes = false;
                }
            } else {
                field.append(character);
            }
        } else if (character == QLatin1Char('"')) {
            inQuotes = true;
        } else if (character == QLatin1Char(',')) {
            record.append(field);
            field.clear();
        } else if (character == QLatin1Char('\n')) {
            consumeRecord();
        } else if (character != QLatin1Char('\r')) {
            field.append(character);
        }
    }

    if (!failed && (!field.isEmpty() || !record.isEmpty())) {
        consumeRecord();
    }

    if (!headerRead || !columns.contains(QStringLiteral("word"))
        || (!columns.contains(QStringLiteral("translation")) && !columns.contains(QStringLiteral("definition")))) {
        database.rollback();
        setLastError(QObject::tr("未找到 ECDICT 所需的 word 和 translation/definition 列。"));
        return false;
    }

    if (failed || !database.commit()) {
        if (!failed) {
            setLastError(database.lastError().text());
        }
        database.rollback();
        return false;
    }

    if (importedCount == 0) {
        setLastError(QObject::tr("词典中没有可用的单词释义。"));
        return false;
    }

    return true;
}

QString DictionaryManager::lookup(const QString &word) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QStringLiteral("SELECT translation FROM dictionary_entries WHERE word = :word COLLATE NOCASE"));
    query.bindValue(QStringLiteral(":word"), word.trimmed());
    if (!query.exec() || !query.next()) {
        return {};
    }

    return query.value(0).toString();
}

QString DictionaryManager::lastError() const
{
    return m_lastError;
}

int DictionaryManager::entryCount() const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM dictionary_entries")) || !query.next()) {
        return 0;
    }
    return query.value(0).toInt();
}

void DictionaryManager::setLastError(const QString &message)
{
    m_lastError = message;
}
