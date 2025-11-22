#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QWidget>
#include "ui_main_window.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectClicked();
    void onSendClicked();

private:
    Ui::MainWindowClass ui;
    void initUI();
    void connectSignals();
};

#endif // MAIN_WINDOW_H
