#pragma once

#include "database/databasemanager.h"
#include "review/reviewscheduler.h"

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
    QWidget *createReviewPage();
    QWidget *createStatisticsPage();
    void loadWords(const QString &keyword = QString());
    void clearWordForm();
    void populateFormFromSelection();
    void addWord();
    void updateWord();
    void deleteWord();
    void showDatabaseError(const QString &action);
    int selectedWordId() const;
    void updateSummary();
    void loadDueWords();
    void showCurrentReviewWord();
    void revealReviewAnswer();
    void rateCurrentWord(ReviewRating rating);
    void setRatingButtonsEnabled(bool enabled);

    DatabaseManager m_database;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_summaryLabel = nullptr;
    QLabel *m_reviewProgressLabel = nullptr;
    QLabel *m_reviewWordLabel = nullptr;
    QLabel *m_reviewAnswerLabel = nullptr;
    QLabel *m_statisticsLabel = nullptr;
    QTableWidget *m_wordsTable = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QLineEdit *m_wordEdit = nullptr;
    QLineEdit *m_definitionEdit = nullptr;
    QTextEdit *m_exampleEdit = nullptr;
    QPushButton *m_updateButton = nullptr;
    QPushButton *m_deleteButton = nullptr;
    QPushButton *m_showAnswerButton = nullptr;
    QPushButton *m_againButton = nullptr;
    QPushButton *m_hardButton = nullptr;
    QPushButton *m_goodButton = nullptr;
    QPushButton *m_easyButton = nullptr;
    QList<WordEntry> m_dueWords;
    int m_currentReviewIndex = -1;
};
