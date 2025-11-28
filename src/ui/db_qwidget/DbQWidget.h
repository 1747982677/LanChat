
#include <QWidget>
#include "ui_DbQWidget.h"

class DbQWidget : public QWidget
{
	Q_OBJECT

public:
	DbQWidget(QWidget *parent = nullptr);
	~DbQWidget();

private slots:
	void onConnectClicked();
	void searchMessages();
	void updateMessages();
	void delMessages();
	void addMessages();

	void createTable();
	void addData();
	void delData();
	void updateData();
	void searchData();
	//void onSearchDbButClicked();
private:
	Ui::DbQWidgetClass uii;
	void initUI();
	void connectSignals();
};

