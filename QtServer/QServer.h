#pragma once
#include "stdafx.h"

#include <QTcpServer>
#include <QTcpSocket>

class QServer : public QTcpServer
{
	Q_OBJECT
public:
	QServer();
	~QServer();

public:
	void init();
	void Run(quint16 port);


protected slots:
	void onNewConnection();
	void onReceiveData();


public:

	bool modifyData(int id, int * value);
	QString getfileName();
	bool checkAuth(QtExMsg * msg);

private:
	void getaccount();
	long getres();
	QString getstrDate();
	QString m_error;
	QMap<QString, QString>  m_map;
};