#pragma once

#include "database/databasemanager.h"

#include <QLabel>
#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupUi();
    void initializeDatabase();

    DatabaseManager m_database;
    QLabel *m_statusLabel = nullptr;
};

