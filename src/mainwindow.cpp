#include "mainwindow.h"
#include "pdf/pdfselectableview.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QPdfDocument>
#include <QPushButton>
#include <QRegularExpression>
#include <QShortcut>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    initializeDatabase();
}

void MainWindow::setupUi()
{
    setWindowTitle(tr("WordFlow"));
    resize(960, 640);

    auto *tabs = new QTabWidget(this);
    setCentralWidget(tabs);

    tabs->addTab(createHomePage(), tr("首页"));
    tabs->addTab(createVocabularyPage(), tr("单词管理"));
    tabs->addTab(createReviewPage(), tr("复习"));
    tabs->addTab(createStatisticsPage(), tr("统计"));
    tabs->addTab(createPdfPage(), tr("PDF 阅读"));

    connect(tabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 2) {
            loadDueWords();
        }
        updateSummary();
    });
}

QWidget *MainWindow::createHomePage()
{
    auto *homePage = new QWidget(this);
    auto *homeLayout = new QVBoxLayout(homePage);

    auto *title = new QLabel(tr("WordFlow 离线背单词"), homePage);
    QFont titleFont = title->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);

    m_summaryLabel = new QLabel(tr("今日复习：0    单词总数：0    今日已复习：0"), homePage);
    m_statusLabel = new QLabel(tr("正在初始化数据库..."), homePage);

    auto *startReviewButton = new QPushButton(tr("开始复习"), homePage);

    homeLayout->addWidget(title);
    homeLayout->addWidget(m_summaryLabel);
    homeLayout->addWidget(startReviewButton);
    homeLayout->addStretch();
    homeLayout->addWidget(m_statusLabel);

    connect(startReviewButton, &QPushButton::clicked, this, [this, tabs = qobject_cast<QTabWidget *>(centralWidget())]() {
        if (tabs) {
            tabs->setCurrentIndex(2);
        }
        loadDueWords();
    });

    return homePage;
}

QWidget *MainWindow::createReviewPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    m_reviewProgressLabel = new QLabel(tr("正在加载今日复习任务…"), page);
    m_reviewWordLabel = new QLabel(page);
    QFont wordFont = m_reviewWordLabel->font();
    wordFont.setPointSize(28);
    wordFont.setBold(true);
    m_reviewWordLabel->setFont(wordFont);
    m_reviewWordLabel->setAlignment(Qt::AlignCenter);
    m_reviewWordLabel->setWordWrap(true);

    m_reviewAnswerLabel = new QLabel(page);
    m_reviewAnswerLabel->setAlignment(Qt::AlignCenter);
    m_reviewAnswerLabel->setWordWrap(true);
    m_reviewAnswerLabel->setMinimumHeight(110);

    m_showAnswerButton = new QPushButton(tr("显示答案"), page);
    m_againButton = new QPushButton(tr("Again\n重置间隔"), page);
    m_hardButton = new QPushButton(tr("Hard\n较短间隔"), page);
    m_goodButton = new QPushButton(tr("Good\n正常间隔"), page);
    m_easyButton = new QPushButton(tr("Easy\n延长间隔"), page);

    auto *ratingLayout = new QHBoxLayout();
    ratingLayout->addWidget(m_againButton);
    ratingLayout->addWidget(m_hardButton);
    ratingLayout->addWidget(m_goodButton);
    ratingLayout->addWidget(m_easyButton);

    layout->addWidget(m_reviewProgressLabel);
    layout->addStretch();
    layout->addWidget(m_reviewWordLabel);
    layout->addWidget(m_reviewAnswerLabel);
    layout->addWidget(m_showAnswerButton, 0, Qt::AlignHCenter);
    layout->addLayout(ratingLayout);
    layout->addStretch();

    connect(m_showAnswerButton, &QPushButton::clicked, this, &MainWindow::revealReviewAnswer);
    connect(m_againButton, &QPushButton::clicked, this, [this]() { rateCurrentWord(ReviewRating::Again); });
    connect(m_hardButton, &QPushButton::clicked, this, [this]() { rateCurrentWord(ReviewRating::Hard); });
    connect(m_goodButton, &QPushButton::clicked, this, [this]() { rateCurrentWord(ReviewRating::Good); });
    connect(m_easyButton, &QPushButton::clicked, this, [this]() { rateCurrentWord(ReviewRating::Easy); });

    setRatingButtonsEnabled(false);
    return page;
}

QWidget *MainWindow::createStatisticsPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    auto *title = new QLabel(tr("学习统计"), page);
    QFont titleFont = title->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);

    m_statisticsLabel = new QLabel(page);
    m_statisticsLabel->setWordWrap(true);
    m_statisticsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    auto *resetProgressButton = new QPushButton(tr("清空学习进度"), page);

    layout->addWidget(title);
    layout->addWidget(m_statisticsLabel);
    layout->addWidget(resetProgressButton, 0, Qt::AlignLeft);
    layout->addStretch();

    connect(resetProgressButton, &QPushButton::clicked, this, &MainWindow::resetStudyProgress);
    return page;
}

QWidget *MainWindow::createPdfPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    auto *title = new QLabel(tr("PDF 阅读与生词收集"), page);
    QFont titleFont = title->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_pdfPathLabel = new QLabel(tr("仅支持带文本层的 PDF 文件"), page);
    m_dictionaryStatusLabel = new QLabel(page);
    m_pdfPathLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_dictionaryStatusLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    auto *importButton = new QPushButton(tr("导入 PDF"), page);
    auto *importDictionaryButton = new QPushButton(tr("导入 ECDICT 词典"), page);
    auto *previousPageButton = new QPushButton(tr("上一页"), page);
    auto *nextPageButton = new QPushButton(tr("下一页"), page);
    auto *zoomOutButton = new QPushButton(tr("缩小"), page);
    auto *fitWidthButton = new QPushButton(tr("适应宽度"), page);
    auto *zoomInButton = new QPushButton(tr("放大"), page);
    auto *zoomLabel = new QLabel(tr("适应宽度"), page);
    m_pdfPageSpinBox = new QSpinBox(page);
    m_pdfPageSpinBox->setPrefix(tr("第 "));
    m_pdfPageSpinBox->setSuffix(tr(" 页"));
    m_pdfPageSpinBox->setEnabled(false);
    auto *addWordButton = new QPushButton(tr("将左侧选词自动加入生词本"), page);

    m_pdfDocument = new QPdfDocument(this);
    m_pdfView = new PdfSelectableView(page);
    m_pdfView->setDocument(m_pdfDocument);
    m_pdfView->setMinimumWidth(420);

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(importButton);
    buttonLayout->addWidget(importDictionaryButton);
    buttonLayout->addWidget(previousPageButton);
    buttonLayout->addWidget(m_pdfPageSpinBox);
    buttonLayout->addWidget(nextPageButton);
    buttonLayout->addWidget(zoomOutButton);
    buttonLayout->addWidget(fitWidthButton);
    buttonLayout->addWidget(zoomInButton);
    buttonLayout->addWidget(zoomLabel);
    buttonLayout->addWidget(addWordButton);
    buttonLayout->addStretch();

    layout->addWidget(title);
    layout->addWidget(m_pdfPathLabel);
    layout->addWidget(m_dictionaryStatusLabel);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_pdfView, 1);

    connect(importButton, &QPushButton::clicked, this, &MainWindow::importPdf);
    connect(importDictionaryButton, &QPushButton::clicked, this, &MainWindow::importEcdictDictionary);
    connect(addWordButton, &QPushButton::clicked, this, &MainWindow::addSelectedPdfWord);
    connect(previousPageButton, &QPushButton::clicked, this, [this]() {
        m_pdfView->setCurrentPage(m_pdfView->currentPage() - 1);
    });
    connect(nextPageButton, &QPushButton::clicked, this, [this]() {
        m_pdfView->setCurrentPage(m_pdfView->currentPage() + 1);
    });
    connect(m_pdfPageSpinBox, &QSpinBox::valueChanged, this, [this](int pageNumber) {
        m_pdfView->setCurrentPage(pageNumber - 1);
    });
    connect(m_pdfView, &PdfSelectableView::currentPageChanged, this, [this](int page) {
        m_pdfPageSpinBox->blockSignals(true);
        m_pdfPageSpinBox->setValue(page + 1);
        m_pdfPageSpinBox->blockSignals(false);
    });
    connect(zoomOutButton, &QPushButton::clicked, this, [this]() {
        m_pdfView->setZoomFactor(m_pdfView->zoomFactor() - 0.25);
    });
    connect(zoomInButton, &QPushButton::clicked, this, [this]() {
        m_pdfView->setZoomFactor(m_pdfView->zoomFactor() + 0.25);
    });
    connect(fitWidthButton, &QPushButton::clicked, m_pdfView, &PdfSelectableView::fitToWidth);
    connect(m_pdfView, &PdfSelectableView::zoomFactorChanged, this, [zoomLabel](qreal factor, bool fitsWidth) {
        zoomLabel->setText(fitsWidth ? tr("适应宽度") : tr("%1%").arg(qRound(factor * 100)));
    });
    auto *zoomInShortcut = new QShortcut(QKeySequence::ZoomIn, page);
    auto *zoomOutShortcut = new QShortcut(QKeySequence::ZoomOut, page);
    connect(zoomInShortcut, &QShortcut::activated, zoomInButton, &QPushButton::click);
    connect(zoomOutShortcut, &QShortcut::activated, zoomOutButton, &QPushButton::click);
    return page;
}

QWidget *MainWindow::createVocabularyPage()
{
    auto *page = new QWidget(this);
    auto *mainLayout = new QHBoxLayout(page);

    auto *formGroup = new QGroupBox(tr("单词信息"), page);
    auto *formLayout = new QVBoxLayout(formGroup);
    auto *fieldsLayout = new QFormLayout();

    m_wordEdit = new QLineEdit(formGroup);
    m_definitionEdit = new QLineEdit(formGroup);
    m_exampleEdit = new QTextEdit(formGroup);
    m_exampleEdit->setFixedHeight(110);

    fieldsLayout->addRow(tr("单词"), m_wordEdit);
    fieldsLayout->addRow(tr("释义"), m_definitionEdit);
    fieldsLayout->addRow(tr("例句"), m_exampleEdit);

    auto *buttonLayout = new QHBoxLayout();
    auto *addButton = new QPushButton(tr("添加"), formGroup);
    m_updateButton = new QPushButton(tr("修改"), formGroup);
    m_deleteButton = new QPushButton(tr("删除"), formGroup);
    auto *clearButton = new QPushButton(tr("清空"), formGroup);

    m_updateButton->setEnabled(false);
    m_deleteButton->setEnabled(false);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(m_updateButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(clearButton);

    formLayout->addLayout(fieldsLayout);
    formLayout->addLayout(buttonLayout);
    formLayout->addStretch();

    auto *listGroup = new QGroupBox(tr("生词本"), page);
    auto *listLayout = new QVBoxLayout(listGroup);
    auto *searchLayout = new QHBoxLayout();

    m_searchEdit = new QLineEdit(listGroup);
    m_searchEdit->setPlaceholderText(tr("搜索单词或释义"));
    auto *searchButton = new QPushButton(tr("搜索"), listGroup);
    auto *showAllButton = new QPushButton(tr("查看全部"), listGroup);

    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(showAllButton);

    m_wordsTable = new QTableWidget(listGroup);
    m_wordsTable->setColumnCount(6);
    m_wordsTable->setHorizontalHeaderLabels({
        tr("ID"),
        tr("单词"),
        tr("释义"),
        tr("例句"),
        tr("下次复习"),
        tr("复习次数"),
    });
    m_wordsTable->setColumnHidden(0, true);
    m_wordsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_wordsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_wordsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_wordsTable->horizontalHeader()->setStretchLastSection(true);
    m_wordsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_wordsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_wordsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    listLayout->addLayout(searchLayout);
    listLayout->addWidget(m_wordsTable);

    mainLayout->addWidget(formGroup, 1);
    mainLayout->addWidget(listGroup, 2);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::addWord);
    connect(m_updateButton, &QPushButton::clicked, this, &MainWindow::updateWord);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::deleteWord);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearWordForm);
    connect(searchButton, &QPushButton::clicked, this, [this]() {
        loadWords(m_searchEdit->text());
    });
    connect(showAllButton, &QPushButton::clicked, this, [this]() {
        m_searchEdit->clear();
        loadWords();
    });
    connect(m_searchEdit, &QLineEdit::returnPressed, this, [this]() {
        loadWords(m_searchEdit->text());
    });
    connect(m_wordsTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::populateFormFromSelection);

    return page;
}

void MainWindow::initializeDatabase()
{
    const QDir appDir(QCoreApplication::applicationDirPath());
    const QString databasePath = appDir.filePath("wordflow.sqlite3");
    const QString dictionaryPath = appDir.filePath("wordflow_dictionary.sqlite3");

    if (!m_database.open(databasePath)) {
        const QString message = tr("数据库初始化失败：%1").arg(m_database.lastError());
        m_statusLabel->setText(message);
        QMessageBox::critical(this, tr("数据库错误"), message);
        return;
    }

    const QString message = tr("数据库已就绪：%1").arg(databasePath);
    m_statusLabel->setText(message);
    statusBar()->showMessage(tr("就绪"));
    if (!m_dictionary.open(dictionaryPath)) {
        statusBar()->showMessage(tr("个人数据库已就绪；本地词典初始化失败"));
    }
    updateDictionaryStatus();
    loadWords();
    loadDueWords();
    updateSummary();
}

void MainWindow::loadWords(const QString &keyword)
{
    if (!m_wordsTable) {
        return;
    }

    const QList<WordEntry> entries = m_database.words(keyword);
    m_wordsTable->setRowCount(entries.size());

    for (int row = 0; row < entries.size(); ++row) {
        const WordEntry &entry = entries.at(row);
        const QString nextReviewDate = entry.nextReviewDate.isValid()
            ? entry.nextReviewDate.toString(Qt::ISODate)
            : QString();

        m_wordsTable->setItem(row, 0, new QTableWidgetItem(QString::number(entry.id)));
        m_wordsTable->setItem(row, 1, new QTableWidgetItem(entry.word));
        m_wordsTable->setItem(row, 2, new QTableWidgetItem(entry.definition));
        m_wordsTable->setItem(row, 3, new QTableWidgetItem(entry.example));
        m_wordsTable->setItem(row, 4, new QTableWidgetItem(nextReviewDate));
        m_wordsTable->setItem(row, 5, new QTableWidgetItem(QString::number(entry.reviewCount)));
    }

    clearWordForm();
    statusBar()->showMessage(tr("已加载 %1 个单词").arg(entries.size()));
}

void MainWindow::clearWordForm()
{
    if (!m_wordEdit) {
        return;
    }

    m_wordsTable->clearSelection();
    m_wordEdit->clear();
    m_definitionEdit->clear();
    m_exampleEdit->clear();
    m_updateButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
    m_wordEdit->setFocus();
}

void MainWindow::populateFormFromSelection()
{
    const int id = selectedWordId();
    const bool hasSelection = id >= 0;
    m_updateButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);

    if (!hasSelection) {
        return;
    }

    const int row = m_wordsTable->currentRow();
    m_wordEdit->setText(m_wordsTable->item(row, 1)->text());
    m_definitionEdit->setText(m_wordsTable->item(row, 2)->text());
    m_exampleEdit->setPlainText(m_wordsTable->item(row, 3)->text());
}

void MainWindow::addWord()
{
    if (m_wordEdit->text().trimmed().isEmpty() || m_definitionEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("输入不完整"), tr("请至少填写单词和释义。"));
        return;
    }

    if (!m_database.addWord(m_wordEdit->text(), m_definitionEdit->text(), m_exampleEdit->toPlainText())) {
        showDatabaseError(tr("添加单词"));
        return;
    }

    loadWords(m_searchEdit->text());
    updateSummary();
    statusBar()->showMessage(tr("已添加单词"));
}

void MainWindow::updateWord()
{
    const int id = selectedWordId();
    if (id < 0) {
        return;
    }

    if (m_wordEdit->text().trimmed().isEmpty() || m_definitionEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("输入不完整"), tr("请至少填写单词和释义。"));
        return;
    }

    if (!m_database.updateWord(id, m_wordEdit->text(), m_definitionEdit->text(), m_exampleEdit->toPlainText())) {
        showDatabaseError(tr("修改单词"));
        return;
    }

    loadWords(m_searchEdit->text());
    updateSummary();
    statusBar()->showMessage(tr("已修改单词"));
}

void MainWindow::deleteWord()
{
    const int id = selectedWordId();
    if (id < 0) {
        return;
    }

    const QString word = m_wordsTable->item(m_wordsTable->currentRow(), 1)->text();
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        tr("确认删除"),
        tr("确定要删除“%1”吗？相关复习记录也会一起删除。").arg(word));

    if (answer != QMessageBox::Yes) {
        return;
    }

    if (!m_database.deleteWord(id)) {
        showDatabaseError(tr("删除单词"));
        return;
    }

    loadWords(m_searchEdit->text());
    updateSummary();
    statusBar()->showMessage(tr("已删除单词"));
}

void MainWindow::showDatabaseError(const QString &action)
{
    QMessageBox::critical(
        this,
        tr("数据库错误"),
        tr("%1失败：%2").arg(action, m_database.lastError()));
}

int MainWindow::selectedWordId() const
{
    if (!m_wordsTable || m_wordsTable->currentRow() < 0) {
        return -1;
    }

    const QTableWidgetItem *idItem = m_wordsTable->item(m_wordsTable->currentRow(), 0);
    return idItem ? idItem->text().toInt() : -1;
}

void MainWindow::updateSummary()
{
    if (!m_summaryLabel) {
        return;
    }

    m_summaryLabel->setText(tr("今日待复习：%1    单词总数：%2    今日已复习：%3")
                                .arg(m_database.dueWordCount())
                                .arg(m_database.totalWordCount())
                                .arg(m_database.reviewedCountOn()));

    if (m_statisticsLabel) {
        m_statisticsLabel->setText(tr("单词总数：%1\n\n累计复习：%2\n\n今日已复习：%3\n\n当前待复习：%4")
                                       .arg(m_database.totalWordCount())
                                       .arg(m_database.totalReviewCount())
                                       .arg(m_database.reviewedCountOn())
                                       .arg(m_database.dueWordCount()));
    }
}

void MainWindow::loadDueWords()
{
    m_dueWords = m_database.dueWords();
    m_sessionReviewAttempts.clear();
    for (const WordEntry &entry : m_dueWords) {
        m_sessionReviewAttempts.insert(entry.id, 1);
    }
    m_currentReviewIndex = m_dueWords.isEmpty() ? -1 : 0;
    showCurrentReviewWord();
}

void MainWindow::showCurrentReviewWord()
{
    if (!m_reviewProgressLabel || !m_reviewWordLabel || !m_reviewAnswerLabel) {
        return;
    }

    if (m_currentReviewIndex < 0 || m_currentReviewIndex >= m_dueWords.size()) {
        m_reviewProgressLabel->setText(tr("今日任务已完成"));
        m_reviewWordLabel->setText(tr("做得好！"));
        m_reviewAnswerLabel->setText(tr("没有到期单词了，可以去单词管理页添加新词。"));
        m_showAnswerButton->setEnabled(false);
        setRatingButtonsEnabled(false);
        return;
    }

    const WordEntry &entry = m_dueWords.at(m_currentReviewIndex);
    m_reviewProgressLabel->setText(tr("今日复习 %1 / %2").arg(m_currentReviewIndex + 1).arg(m_dueWords.size()));
    m_reviewWordLabel->setText(entry.word);
    m_reviewAnswerLabel->clear();
    m_showAnswerButton->setEnabled(true);
    setRatingButtonsEnabled(false);
}

void MainWindow::revealReviewAnswer()
{
    if (m_currentReviewIndex < 0 || m_currentReviewIndex >= m_dueWords.size()) {
        return;
    }

    const WordEntry &entry = m_dueWords.at(m_currentReviewIndex);
    QString answer = entry.definition;
    if (!entry.example.trimmed().isEmpty()) {
        answer += QStringLiteral("\n\n") + tr("例句：") + entry.example;
    }
    m_reviewAnswerLabel->setText(answer);
    m_showAnswerButton->setEnabled(false);
    setRatingButtonsEnabled(true);
}

void MainWindow::rateCurrentWord(ReviewRating rating)
{
    if (m_currentReviewIndex < 0 || m_currentReviewIndex >= m_dueWords.size()) {
        return;
    }

    const WordEntry &entry = m_dueWords.at(m_currentReviewIndex);
    const ReviewSchedule schedule = ReviewScheduler::schedule(
        rating,
        entry.intervalDays,
        entry.easeFactor,
        entry.lapseCount,
        entry.lastReviewDate);
    if (!m_database.recordReview(entry.id,
                                 static_cast<int>(rating),
                                 schedule.nextReviewDate,
                                 schedule.intervalDays,
                                 schedule.easeFactor,
                                 schedule.lapseCount)) {
        showDatabaseError(tr("保存复习记录"));
        return;
    }

    const int attempts = m_sessionReviewAttempts.value(entry.id, 1);
    const bool shouldRepeatInSession = rating != ReviewRating::Easy
        && attempts < kMaximumSessionAttempts;
    if (shouldRepeatInSession) {
        m_sessionReviewAttempts.insert(entry.id, attempts + 1);
        WordEntry repeatedEntry = entry;
        repeatedEntry.nextReviewDate = schedule.nextReviewDate;
        repeatedEntry.intervalDays = schedule.intervalDays;
        repeatedEntry.easeFactor = schedule.easeFactor;
        repeatedEntry.lapseCount = schedule.lapseCount;
        repeatedEntry.lastReviewDate = QDate::currentDate();
        m_dueWords.append(repeatedEntry);
    }

    ++m_currentReviewIndex;
    loadWords(m_searchEdit ? m_searchEdit->text() : QString());
    updateSummary();
    showCurrentReviewWord();

    QString message = tr("已安排 %1 天后复习：%2（当前记忆保持估计 %3%）")
                          .arg(schedule.intervalDays)
                          .arg(schedule.nextReviewDate.toString(Qt::ISODate))
                          .arg(qRound(schedule.retrievability * 100));
    if (shouldRepeatInSession) {
        message += tr("；本轮将再次复习（%1/%2）")
                       .arg(attempts + 1)
                       .arg(kMaximumSessionAttempts);
    } else if (rating != ReviewRating::Easy && attempts >= kMaximumSessionAttempts) {
        message += tr("；已达到本轮重复上限");
    }
    statusBar()->showMessage(message);
}

void MainWindow::setRatingButtonsEnabled(bool enabled)
{
    m_againButton->setEnabled(enabled);
    m_hardButton->setEnabled(enabled);
    m_goodButton->setEnabled(enabled);
    m_easyButton->setEnabled(enabled);
}

void MainWindow::resetStudyProgress()
{
    const QMessageBox::StandardButton answer = QMessageBox::warning(
        this,
        tr("确认清空学习进度"),
        tr("将删除所有复习记录，并把所有单词恢复为今日待复习、复习次数为 0。生词本不会被删除。是否继续？"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (answer != QMessageBox::Yes) {
        return;
    }

    if (!m_database.resetStudyProgress()) {
        showDatabaseError(tr("清空学习进度"));
        return;
    }

    loadWords(m_searchEdit ? m_searchEdit->text() : QString());
    loadDueWords();
    updateSummary();
    statusBar()->showMessage(tr("学习进度已清空，所有单词均可重新复习"));
}

void MainWindow::importPdf()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this, tr("选择 PDF 文件"), QString(), tr("PDF 文件 (*.pdf)"));
    if (filePath.isEmpty()) {
        return;
    }

    if (!m_pdfDocument || !m_pdfView) {
        return;
    }

    m_pdfDocument->close();
    if (m_pdfDocument->load(filePath) != QPdfDocument::Error::None) {
        QMessageBox::warning(this, tr("无法导入 PDF"), tr("文件无法读取、格式无效或需要密码。"));
        return;
    }

    bool hasTextLayer = false;
    for (int page = 0; page < m_pdfDocument->pageCount(); ++page) {
        if (!m_pdfDocument->getAllText(page).text().trimmed().isEmpty()) {
            hasTextLayer = true;
            break;
        }
    }

    if (!hasTextLayer) {
        QMessageBox::information(
            this,
            tr("未提取到文本"),
            tr("该 PDF 没有可提取的文本层，扫描件需要 OCR，当前版本暂不支持。"));
        return;
    }

    m_currentPdfPath = filePath;
    m_pdfView->setDocument(m_pdfDocument);
    m_pdfPageSpinBox->setRange(1, m_pdfDocument->pageCount());
    m_pdfPageSpinBox->setValue(1);
    m_pdfPageSpinBox->setEnabled(true);
    m_pdfPathLabel->setText(tr("当前文件：%1（%2 页）")
                                .arg(QFileInfo(filePath).fileName())
                                .arg(m_pdfDocument->pageCount()));
    statusBar()->showMessage(tr("已导入 PDF：%1 页").arg(m_pdfDocument->pageCount()));
}

void MainWindow::importEcdictDictionary()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this, tr("选择 ECDICT CSV 词典"), QString(), tr("CSV 文件 (*.csv)"));
    if (filePath.isEmpty()) {
        return;
    }

    if (!m_dictionary.importEcdictCsv(filePath)) {
        QMessageBox::warning(this, tr("导入词典失败"), m_dictionary.lastError());
        return;
    }

    updateDictionaryStatus();
    statusBar()->showMessage(tr("ECDICT 词典导入完成：%1 条").arg(m_dictionary.entryCount()));
}

void MainWindow::updateDictionaryStatus()
{
    if (!m_dictionaryStatusLabel) {
        return;
    }

    const int count = m_dictionary.entryCount();
    if (count > 0) {
        m_dictionaryStatusLabel->setText(tr("本地词典：已导入 %1 条词条；选词后将自动填入中文释义。").arg(count));
    } else {
        m_dictionaryStatusLabel->setText(
            tr("本地词典：未导入。点击“导入 ECDICT 词典”选择 ecdict.mini.csv 或 ecdict.csv。"));
    }
}

void MainWindow::addSelectedPdfWord()
{
    if (!m_pdfView || m_currentPdfPath.isEmpty()) {
        QMessageBox::information(this, tr("请先导入 PDF"), tr("请先导入一个带文本层的 PDF 文件。"));
        return;
    }

    QString word = m_pdfView ? m_pdfView->selectedText() : QString();
    word.replace(QChar::ParagraphSeparator, QChar::Space);
    word = word.trimmed();
    const QRegularExpression englishWord(QStringLiteral("^[A-Za-z]+(?:['-][A-Za-z]+)*$"));
    if (!englishWord.match(word).hasMatch()) {
        QMessageBox::information(this, tr("请选择单个英文单词"), tr("请在左侧 PDF 页面中拖选一个英文单词后再加入生词本。"));
        return;
    }

    QString definition = m_dictionary.lookup(word);
    if (definition.isEmpty()) {
        bool accepted = false;
        definition = QInputDialog::getMultiLineText(
            this,
            tr("未找到自动释义"),
            tr("本地词典未收录“%1”，请手动填写释义：").arg(word),
            QString(),
            &accepted);
        if (!accepted || definition.trimmed().isEmpty()) {
            return;
        }
    }

    const QString source = tr("PDF：%1").arg(QFileInfo(m_currentPdfPath).fileName());
    if (m_database.wordExists(word)) {
        QMessageBox::information(
            this,
            tr("单词已存在"),
            tr("“%1”已在生词本中，无需重复添加。").arg(word));
        statusBar()->showMessage(tr("生词本已有单词：%1").arg(word));
        return;
    }

    if (!m_database.addWord(word, definition, QString(), source)) {
        showDatabaseError(tr("添加 PDF 生词"));
        return;
    }

    loadWords(m_searchEdit ? m_searchEdit->text() : QString());
    updateSummary();
    statusBar()->showMessage(tr("已从 PDF 加入生词本：%1").arg(word));
}
