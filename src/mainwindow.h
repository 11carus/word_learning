#pragma once

#include "database/databasemanager.h"
#include "dictionary/dictionarymanager.h"
#include "review/reviewscheduler.h"

#include <QLabel>
#include <QHash>
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
    QWidget *createPdfPage();
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
    void resetStudyProgress();
    void importPdf();
    void importEcdictDictionary();
    void updateDictionaryStatus();
    void addSelectedPdfWord();

    DatabaseManager m_database;
    DictionaryManager m_dictionary;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_summaryLabel = nullptr;
    QLabel *m_reviewProgressLabel = nullptr;
    QLabel *m_reviewWordLabel = nullptr;
    QLabel *m_reviewAnswerLabel = nullptr;
    QLabel *m_statisticsLabel = nullptr;
    QLabel *m_pdfPathLabel = nullptr;
    QLabel *m_dictionaryStatusLabel = nullptr;
    QTableWidget *m_wordsTable = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QLineEdit *m_wordEdit = nullptr;
    QLineEdit *m_definitionEdit = nullptr;
    QTextEdit *m_exampleEdit = nullptr;
    class QPdfDocument *m_pdfDocument = nullptr;
    class PdfSelectableView *m_pdfView = nullptr;
    class QSpinBox *m_pdfPageSpinBox = nullptr;
    QPushButton *m_updateButton = nullptr;
    QPushButton *m_deleteButton = nullptr;
    QPushButton *m_showAnswerButton = nullptr;
    QPushButton *m_againButton = nullptr;
    QPushButton *m_hardButton = nullptr;
    QPushButton *m_goodButton = nullptr;
    QPushButton *m_easyButton = nullptr;
    QList<WordEntry> m_dueWords;                // 本轮复习队列；未掌握的单词会再次追加到末尾。
    QHash<int, int> m_sessionReviewAttempts;    // 单词 ID 到本轮已出现次数的映射。
    int m_currentReviewIndex = -1;              // 当前单词在复习队列中的索引；-1 表示没有任务。
    static constexpr int kMaximumSessionAttempts = 3; // 单词在一轮中的最大出现次数。
    QString m_currentPdfPath;                   // 当前打开的 PDF 文件路径。
};
