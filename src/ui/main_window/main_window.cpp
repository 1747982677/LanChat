#include "main_window.h"
#include "utils/logger.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    initUI();
    connectSignals();
    
    Logger::getInstance().log("MainWindow created");
}

MainWindow::~MainWindow()
{
    Logger::getInstance().log("MainWindow destroyed");
}

void MainWindow::initUI()
{
    // Set window title
    setWindowTitle("LanChat - LAN Chat Tool");
    
    // TODO: Initialize UI components
}

void MainWindow::connectSignals()
{
    // TODO: Connect signals and slots
}

void MainWindow::onConnectClicked()
{
    Logger::getInstance().log("Connect button clicked");
    // TODO: Implement connection logic
}

void MainWindow::onSendClicked()
{
    Logger::getInstance().log("Send button clicked");
    // TODO: Implement send message logic
}
