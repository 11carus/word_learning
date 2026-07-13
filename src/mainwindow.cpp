#include "mainwindow.h"

#include <QCoreApplication>
#include <QDir>
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

    auto *homePage = new QWidget(tabs);
    auto *homeLayout = new QVBoxLayout(homePage);

    auto *title = new QLabel(tr("WordFlow 离线背单词"), homePage);
    QFont titleFont = title->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto *summary = new QLabel(tr("今日复习：0    单词总数：0    今日已复习：0"), homePage);
    m_statusLabel = new QLabel(tr("正在初始化数据库..."), homePage);

    auto *startReviewButton = new QPushButton(tr("开始复习"), homePage);
    startReviewButton->setEnabled(false);

    homeLayout->addWidget(title);
    homeLayout->addWidget(summary);
    homeLayout->addWidget(startReviewButton);
    homeLayout->addStretch();
    homeLayout->addWidget(m_statusLabel);

    tabs->addTab(homePage, tr("首页"));
    tabs->addTab(new QLabel(tr("单词管理页面将在下一步实现。"), tabs), tr("单词管理"));
    tabs->addTab(new QLabel(tr("复习卡片页面将在下一步实现。"), tabs), tr("复习"));
    tabs->addTab(new QLabel(tr("学习统计页面将在下一步实现。"), tabs), tr("统计"));
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
}

