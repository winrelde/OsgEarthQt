#pragma once
#include <QtWidgets/QDialog>
#include "ui_Information.h"

class Information : public QDialog
{
	Q_OBJECT

public:

	Information(QWidget *parent = Q_NULLPTR);
	~Information() {}

private slots:

	void on_btnOK();

private:

	Ui::Information	ui;
};
