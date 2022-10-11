#include <Windows.h>
#include "Common.h"

#include "MyOSGEarthQT.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MyOSGEarthQT w;
	w.show();
	return a.exec();
}
