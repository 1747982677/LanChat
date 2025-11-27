#pragma once

#include <QWidget>
#include "ui_MessageList.h"

class MessageList : public QWidget
{
	Q_OBJECT

public:
	MessageList(QWidget *parent = nullptr);
	~MessageList();

private:
	Ui::MessageListClass ui;
};

