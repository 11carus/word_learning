#pragma once

#include "database/databasemanager.h"

#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupUi();
    void initializeDatabase();
    QWidget *createHomePage();
    QWidget *createVocabularyPage();
    void loadWords(const QString &keyword = QString());
    void clearWordForm();
    void populateFormFromSelection();
    void addWord();
    void updateWord();
    void deleteWord();
    void showDatabaseError(const QString &action);
    int selectedWordId() const;
    void updateSummary();

    DatabaseManager m_database;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_summaryLabel = nullptr;
    QTableWidget *m_wordsTable = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QLineEdit *m_wordEdit = nullptr;
    QLineEdit *m_definitionEdit = nullptr;
    QTextEdit *m_exampleEdit = nullptr;
    QPushButton *m_updateButton = nullptr;
    QPushButton *m_deleteButton = nullptr;
};
