#pragma once
#include <QtWidgets/QDialog>
#include "ui_ViewshedPara.h"

class ViewshedPara : public QDialog
{
	Q_OBJECT

public:

	ViewshedPara(QWidget *parent = Q_NULLPTR);
	~ViewshedPara() {}

private slots:

	void on_btnOK();
	void on_btnCancel();

private:

	Ui::ViewshedPara	ui;
	int					m_lastSpokes;
	int					m_lastSegment;
};
