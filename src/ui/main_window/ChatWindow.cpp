// ChatWindow.cpp
/*
 * 页面名称：聊天页面
 * 功能：显示当前用户的聊天会话
 */
#include "ChatWindow.h"
#include <QLabel>
#include <QVBoxLayout>

//聊天框占位

ChatWindow::ChatWindow(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->setMinimumWidth(300);
}

ChatWindow::~ChatWindow()
{}

