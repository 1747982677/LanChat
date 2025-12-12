#pragma once

#include <QWidget>
#include <QMap>
#include <QString>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QJsonArray>
#include "ui_ContactList.h"

class ContactItem;

class ContactList : public QWidget
{
	Q_OBJECT

public:
	ContactList(QWidget *parent = nullptr);
	~ContactList();
	
	// 加载联系人列表
	void loadContacts();
	
	// 搜索联系人（模糊搜索昵称和备注）
	void searchContacts(const QString& keyword);

private slots:
	// 处理联系人列表加载完成
	void onContactListLoaded(const QJsonArray& contacts);

private:
	void setupUi();
	void clearContacts();
	void displayContacts(const QJsonArray& contacts);  // 显示联系人列表

private:
	Ui::ContactListClass ui;
	QMap<QString, ContactItem*> m_itemsById; // map userId -> item
	QWidget* m_container;  // 滚动区域的内容容器
	QVBoxLayout* m_containerLayout;  // 容器的布局
	QScrollArea* m_scrollArea;  // 滚动区域
	QJsonArray m_allContacts;  // 保存所有联系人数据（用于搜索）
};

