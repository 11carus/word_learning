#include "mainwindow.h"

#include <QCoreApplication>
#include <QDir>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
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
    tabs->addTab(new QLabel(tr("复习卡片页面将在下一步实现。"), tabs), tr("复习"));
    tabs->addTab(new QLabel(tr("学习统计页面将在下一步实现。"), tabs), tr("统计"));
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
    startReviewButton->setEnabled(false);

    homeLayout->addWidget(title);
    homeLayout->addWidget(m_summaryLabel);
    homeLayout->addWidget(startReviewButton);
    homeLayout->addStretch();
    homeLayout->addWidget(m_statusLabel);

    return homePage;
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

    if (!m_database.open(databasePath)) {
        const QString message = tr("数据库初始化失败：%1").arg(m_database.lastError());
        m_statusLabel->setText(message);
        QMessageBox::critical(this, tr("数据库错误"), message);
        return;
    }

    const QString message = tr("数据库已就绪：%1").arg(databasePath);
    m_statusLabel->setText(message);
    statusBar()->showMessage(tr("就绪"));
    loadWords();
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

    m_summaryLabel->setText(tr("今日复习：0    单词总数：%1    今日已复习：0")
                                .arg(m_database.totalWordCount()));
}
