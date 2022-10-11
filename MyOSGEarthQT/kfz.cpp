#include <Windows.h>
#include "Common.h"

#include "kfz.h"
#include <QtGui/QIntValidator>

kfz::kfz(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.okButton, &QPushButton::clicked, this, &kfz::on_btnOK);
}

void kfz::on_btnOK()
{
	close();
}
