#pragma once

#include <QWidget>
#include "ui_NewFriendList.h"
#include <QScrollArea>
#include <QVBoxLayout>

class NewFriendList : public QWidget
{
	Q_OBJECT

public:
	NewFriendList(QWidget *parent = nullptr);
	~NewFriendList();
	void loadData();
	void acceptNewFriendAsk(const QString& name);

private:
	QScrollArea* m_scroll;         // 滚动区域
	QWidget* m_container;          // 滚动区域容器
	QVBoxLayout* m_containerLayout; // 容器布局

	Ui::NewFriendListClass ui;
};

