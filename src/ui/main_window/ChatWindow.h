#pragma once

#include <QWidget>
#include <QJsonObject>
#include "ui_ChatWindow.h"

class ChatWindow : public QWidget
{
	Q_OBJECT

public:
	ChatWindow(QWidget *parent = nullptr);
	~ChatWindow();

private slots:
	void onMessageReceived(const QJsonObject &msgJson, const QString &from);

private:
	Ui::ChatWindowClass ui;
};

