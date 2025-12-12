#pragma once

#include <QWidget>
#include "ui_personinfo.h"

class personinfo : public QWidget
{
	Q_OBJECT

public:
	personinfo(QWidget *parent = nullptr);
	~personinfo();

private:
	Ui::personinfoClass ui;
};

