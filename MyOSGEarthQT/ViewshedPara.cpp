#include <Windows.h>
#include "Common.h"

#include "ViewshedPara.h"
#include "MyOSGEarthQT.h"

#include <QtGui/QIntValidator>

ViewshedPara::ViewshedPara(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	ui.lineEdit_1->setValidator(new QIntValidator(10, 10000,this));
	ui.lineEdit_1->setText("200"); 
	m_lastSpokes = 200;
	ui.lineEdit_2->setValidator(new QIntValidator(10, 10000, this));
	ui.lineEdit_2->setText("200");
	m_lastSegment = 200;

	connect(ui.buttonOK, &QPushButton::clicked, this, &ViewshedPara::on_btnOK);
	connect(ui.buttonCancel, &QPushButton::clicked, this, &ViewshedPara::on_btnCancel);
}

void ViewshedPara::on_btnOK()
{
	close();
	MyOSGEarthQT* parent = (MyOSGEarthQT*)parentWidget();
	parent->sendViewshedPara(ui.lineEdit_1->text().toInt(), ui.lineEdit_2->text().toInt());
	m_lastSpokes = ui.lineEdit_1->text().toInt();
	m_lastSegment = ui.lineEdit_2->text().toInt();
}

void ViewshedPara::on_btnCancel()
{
	close();
	ui.lineEdit_1->setText(QString::number(m_lastSpokes));
	ui.lineEdit_2->setText(QString::number(m_lastSegment));
}