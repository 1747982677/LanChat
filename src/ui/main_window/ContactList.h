#pragma once

#include <QWidget>
#include "ui_ContactList.h"

class ContactList : public QWidget
{
	Q_OBJECT

public:
	ContactList(QWidget *parent = nullptr);
	~ContactList();

private:
	Ui::ContactListClass ui;
};

