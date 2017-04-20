#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_qtguiapplication2.h"
#include <QTcpSocket>


class QtGuiApplication2 : public QMainWindow
{
    Q_OBJECT

public:
    QtGuiApplication2(QWidget *parent = Q_NULLPTR);


public:

	void sendMessage(int id, const int* value, QString authData);

private slots:
	void on_pushButtonSubmit_clicked();

	void onReadyRead();

	void onError(QAbstractSocket::SocketError socketError);

	void ConnectToServer(const char *ip, quint16 port);

private:
    Ui::QtGuiApplication2Class ui;
	QTcpSocket m_sock;
	int m_baseDay;

};
