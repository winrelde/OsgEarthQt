#pragma once
#include <QtWidgets/QDialog>
#include "ui_kfz.h"

class kfz : public QDialog
{
	Q_OBJECT

public:

	kfz(QWidget *parent = Q_NULLPTR);
	~kfz() {}

private slots:

	void on_btnOK();

private:

	Ui::kfz	ui;
};
