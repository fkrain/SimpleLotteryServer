#include <QtCore/QCoreApplication>
#include "stdafx.h"
#include "QServer.h"
int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	auto obj = new QServer;

//	obj->modifyData(2, 12345);
	return a.exec();
}
