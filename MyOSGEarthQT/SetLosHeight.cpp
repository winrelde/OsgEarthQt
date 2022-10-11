#include <Windows.h>
#include "Common.h"

#include "SetLosHeight.h"
#include "MyOSGEarthQT.h"

#include <QtGui/QDoubleValidator>

SetLosHeight::SetLosHeight(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	ui.lineEdit->setValidator(new QDoubleValidator(0.0,8848,6,this));//文本框限制，十进制
	ui.lineEdit->setText("2.0");
	m_lastHeight = 2.0;//初始
	connect(ui.buttonOK, &QPushButton::clicked, this, &SetLosHeight::on_btnOK);
	connect(ui.buttonCancel, &QPushButton::clicked, this, &SetLosHeight::on_btnCancel);
}

//点击确定设置视高
void SetLosHeight::on_btnOK()
{
	close();
	MyOSGEarthQT* parent = (MyOSGEarthQT*)parentWidget();
	parent->sendLosHeight(ui.lineEdit->text().toFloat());
	m_lastHeight = ui.lineEdit->text().toFloat();
}

//点击取消还是那个初始值
void SetLosHeight::on_btnCancel()
{
	close();
	ui.lineEdit->setText(QString::number(m_lastHeight));
}