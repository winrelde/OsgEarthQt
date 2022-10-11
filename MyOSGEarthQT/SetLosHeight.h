#pragma once
#include <QtWidgets/QDialog>
#include "ui_SetLosHeight.h"

class SetLosHeight : public QDialog
{
	Q_OBJECT

public:

	SetLosHeight(QWidget *parent = Q_NULLPTR);
	~SetLosHeight() {}

private slots:

	void on_btnOK();
	void on_btnCancel();

private:

	Ui::SetLosHeight	ui;
	double				m_lastHeight;
};
