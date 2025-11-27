#pragma once

#include <QWidget>
#include "ui_ChatWindow.h"

class ChatWindow : public QWidget
{
	Q_OBJECT

public:
	ChatWindow(QWidget *parent = nullptr);
	~ChatWindow();

private:
	Ui::ChatWindowClass ui;
};

