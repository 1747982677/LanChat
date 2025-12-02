#pragma once

#include <QWidget>
#include <QMap>
#include <QString>
#include "ui_ContactList.h"

class ContactItem;

class ContactList : public QWidget
{
	Q_OBJECT

public:
	ContactList(QWidget *parent = nullptr);
	~ContactList();

private:
	Ui::ContactListClass ui;
	QMap<QString, ContactItem*> m_itemsById; // map userId -> item
};

