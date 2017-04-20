#include "stdafx.h"
#include "qtguiapplication2.h"
#include <QMessagebox>
#include <QHostAddress>


QtGuiApplication2::QtGuiApplication2(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connect(&m_sock, &QTcpSocket::readyRead, this, &QtGuiApplication2::onReadyRead);
	
}

bool checkHourseNum(const int *arr)
{
	int check[11] = { 0 };

	for (int i = 0; i < 10 ; i++)
	{
		if (arr[i] < 0 || arr[i] > 10 )
		{
			return false;
		}
		check[arr[i]] = 1;
	}
	for (int i = 1; i < 11 ; i++)
	{
        if (!check[i])
        {
			return false;
        }
	}
	return true;
}

void QtGuiApplication2::on_pushButtonSubmit_clicked()
{
	auto inputID = ui.textEditID;

	int hourseArry[] = { ui.textEditValue_1->toPlainText().toInt(),
		ui.textEditValue_2->toPlainText().toInt(),
		ui.textEditValue_3->toPlainText().toInt(),
		ui.textEditValue_4->toPlainText().toInt(),
		ui.textEditValue_5->toPlainText().toInt(),
		ui.textEditValue_6->toPlainText().toInt(),
		ui.textEditValue_7->toPlainText().toInt(),
		ui.textEditValue_8->toPlainText().toInt(),
		ui.textEditValue_9->toPlainText().toInt(),
		ui.textEditValue_10->toPlainText().toInt()};
	
	auto strid = inputID->toPlainText();
	auto strvalue = hourseArry;
	m_baseDay = 0;
	
	bool bok = false;
	int id  =  strid.toInt(&bok);
	//id--;
	// 获取基础res
	auto date = QDate::currentDate();
	int iDay = date.day() - 1;
	m_baseDay = iDay * 190 + 1;
	if (!bok || id < m_baseDay || id > m_baseDay + 190)
	{
		QMessageBox::warning(this, "输入期数不合法", "输入期数不合法, 检查输入数据");
		return;
	}
	id -= m_baseDay;
	if (!bok || !checkHourseNum(hourseArry))
	{
		QMessageBox::warning(this, "输入赛马错误", "输入错误,赛马顺序为 1- 10 ,不可重复, 请检查");
		return;
	}
	auto authData = ui.textEditAuth->toPlainText();
	if (authData.isEmpty(), authData.length()> 500)
	{
		QMessageBox::warning(this, "输入错误", "请输入授权码, 或授权码有误");
		return;
	}


	sendMessage(id, hourseArry, authData);
	 
}

void QtGuiApplication2::sendMessage(int id, const int* value, QString authData)
{
	char ip[] = { "121.43.63.74" };
	//char ip[] = { "127.0.0.1" };
	ConnectToServer(ip, 3000);
	bool bok = m_sock.waitForConnected();
	if (!bok)
	{
		QMessageBox::warning(this, "提交结果", QString("服务器连接超时"));
		return;
	}

	QtExMsg msg;
	memset(&msg, 0, sizeof msg);
	memcpy(msg.buf, authData.toStdString().c_str(), authData.length() + 1);

	msg.head = 0x5065;
	msg.id = id;
	/*msg.value = value;*/
	memcpy(msg.hourseArry, value, sizeof (int)  * 10);

	auto ret = m_sock.write((char*)&msg, sizeof QtExMsg);
	if (0 >= ret)
		QMessageBox::warning(this, "提交结果", QString("数据写入失败"));
	auto res = m_sock.waitForBytesWritten();
  
	if (!res)
	{
		QMessageBox::warning(this, "提交结果", QString("数据写入超时"));
	}

}

void QtGuiApplication2::onReadyRead()
{
	char buf[1024] = { 0 };
	 m_sock.read(buf, 1024);
	 QString resalt = buf;
	QMessageBox::information(this, "提交结果", resalt);
	return;
}

void QtGuiApplication2::onError(QAbstractSocket::SocketError socketError)
{
	QMessageBox::warning(this, "提交结果", QString("服务器连接失败,检查网络环境, 错误码: %1").arg(socketError));
	return;
}

void QtGuiApplication2::ConnectToServer(const char * ip, quint16 port)
{
	QString strip(ip);
	QHostAddress addr;
	addr.setAddress(strip);
	m_sock.connectToHost(addr, port);
}
