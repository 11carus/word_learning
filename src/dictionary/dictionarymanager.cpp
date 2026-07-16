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

    QString field;                 // 正在解析的当前字段内容。
    QStringList record;            // 当前一行已经解析出的全部字段。
    QHash<QString, int> columns;   // 规范化表头名称到列号的映射。
    bool inQuotes = false;         // 当前字符是否位于双引号包围的字段内。
    bool headerRead = false;       // 是否已经读取并解析第一行表头。
    int importedCount = 0;         // 成功写入数据库的有效词条数量。
    bool failed = false;           // 任一 SQL 插入失败后终止后续扫描。

    // 完成一行 CSV 记录：第一行建立“列名 -> 列号”的映射，
    // 后续记录再根据表头位置提取 word 和 translation/definition。
    // 这样即使 ECDICT 的列顺序发生变化，导入逻辑仍然有效。
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

    // 单次扫描 CSV 的有限状态机。
    // inQuotes == false：逗号结束字段，换行结束记录；
    // inQuotes == true ：逗号和换行都是字段内容，连续两个双引号表示
    //                    字段内的一个双引号。
    // 因此这里不能简单使用 split(',')，否则会错误拆分带逗号的释义。
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
