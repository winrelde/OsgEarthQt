#include <Windows.h>
#include "Common.h"

#include "Information.h"
#include "MyOSGEarthQT.h"

#include <QtGui/QIntValidator>

Information::Information(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.buttonOK, &QPushButton::clicked, this, &Information::on_btnOK);
}

void ViewshedPara::on_btnOK()
{
	close();
}
