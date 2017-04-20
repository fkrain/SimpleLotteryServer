#include "QServer.h"
#include <QHostAddress>
#include <QDataStream>
#include <QFile>
#include <qt_windows.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDate>
bool checkHourseNum(const int *arr)
{
	int check[11] = { 0 };

	for (int i = 0; i < 10; i++)
	{
		if (arr[i] < 0 || arr[i] > 10)
		{
			return false;
		}
		check[arr[i]] = 1;
	}
	for (int i = 1; i < 11; i++)
	{
		if (!check[i])
		{
			return false;
		}
	}
	return true;
}

QServer::QServer()
{
	getaccount();
	init();
}

QServer::~QServer()
{
}

void QServer::init()
{
	Run(3000);
	bool ret = connect(this, &QTcpServer::newConnection, this, &QServer::onNewConnection);
	if (ret == 0)
	{
		qDebug() << ("启动失败, 请重新启动");
	}
}

void QServer::Run(quint16 port)
{

	if (this->listen(QHostAddress::Any, port))
		qDebug() << ("开始监听,启动成功 !");
	else
		qDebug() << ("端口被占用,请检查是否启动了多个本程序 !");
}

void QServer::onNewConnection()
{
	auto sock = nextPendingConnection();

	bool ret = connect(sock, &QTcpSocket::readyRead, this, &QServer::onReceiveData);
}

void QServer::onReceiveData()
{
	auto sock = (QTcpSocket *)sender();
	char buf[1024] = { 0 };
	sock->read(buf, 1024);
	QtExMsg *msg = (QtExMsg *)buf;
	if (msg->head == 0x5065)
	{
		if (!checkAuth(msg))
		{
			QString str = "授权过期或授权码错误!";
			sock->write(str.toUtf8());
			sock->waitForBytesWritten();
			sock->close();
			return;
		}
		auto res = getres();
		int tmp = res - 1;
		if (tmp != msg->id)
		{
			auto date = QDate::currentDate();
			int iDay = date.day() - 1;
			auto baseDay = iDay * 190 ;
			QString str = QString("只能修改当前期数, 当前期数为 %1 !").arg(baseDay + res);
			sock->write(str.toUtf8());
			sock->waitForBytesWritten();
			sock->close();
			return;
		}
		if (!checkHourseNum(msg->hourseArry))
		{
			QString str = QString("赛马排名数据有误  !");
			sock->write(str.toUtf8());
			sock->waitForBytesWritten();
			sock->close();
			return;
		}
		bool ret = modifyData(msg->id, msg->hourseArry);
		if (ret)
		{
			QString str = "数据修改成功";
			sock->write(str.toUtf8());
			if (sock->waitForBytesWritten())
			{
				qDebug() << ("修改 ok!");
				sock->close();
			}
			else
			{
				qDebug() << ("send failed!");
			}
		}
		else
		{
			qDebug() << "数据修改失败";
		}

	}
	else
	{
		auto sock = (QTcpSocket*)sender();
		QString str = "数据修改失败,可能网络波动, 或者数据格式有误,请检查后重新发送!";
		sock->write(str.toUtf8());
		sock->waitForBytesWritten();
		sock->close();
	}

}


bool QServer::modifyData(int id, int * value)
{
	QString strFileName = getfileName();
	QFile file(strFileName);
	bool  res = file.open(QIODevice::ReadWrite);
	if (!res)
	{
		qDebug() << "文件: "<<strFileName<< ("  打开失败!");
		return false;
	}
	auto doc = QJsonDocument::fromJson(file.readAll());

	if (!doc.isObject())
	{
		return false;
	}
	auto& jsonObj = doc.object();

	auto  arrDataValue = jsonObj.value("data");

	if (!arrDataValue.isArray())
	{
		return false;
	}

	auto &arr = arrDataValue.toArray();
	auto &valueRef = arr[id];
	QJsonObject obj = valueRef.toObject();
	QString strVar;

	for (int i = 0; i < 9; i++)
	{
		
		strVar += QString::number(value[i]) + ",";
	}
	strVar += QString::number(value[9]);
// 	QString strVar;
// 	int lens = strArr.length();
// 	lens = 5 - lens;
// 	QString tst1;
// 
// 
// 	for (int i = 0; i < lens; i++)
// 	{
// 		tst1 += "0";
// 	}
// 	strArr = tst1 + strArr;
// 
// 	strVar = strArr[0];
// 	strVar += ",";
// 	strVar += strArr[1];
// 	strVar += ",";
// 	strVar += strArr[2];
// 	strVar += ",";
// 	strVar += strArr[3];
// 	strVar += ",";
// 	strVar += strArr[4];

	if (obj.contains("vol"))
	{
		auto vol = obj.take("vol").toInt();
		qDebug() << "修改期数" << vol;
		obj["vol"] = vol;
	}

	if (obj.contains("res"))
	{
		auto res = obj.take("res").toString();
		qDebug() << "src:" << res;
		qDebug() << "des:" << strVar;
		file.close();

		file.open(QIODevice::WriteOnly | QIODevice::Truncate);
		obj["res"] = strVar;
		arr[id] = obj;
		jsonObj["data"] = arr;
		doc.setObject(jsonObj);
		file.write(doc.toJson());
		file.close();
		return true;

	}
	else
	{
		qDebug() << "no container res";
	}


	return false;
}

QString QServer::getfileName()
{
	auto date = QDate::currentDate();
	int iYear = date.year();
	int iMon = date.month();
	int iDay = date.day();
	SYSTEMTIME st, lt;
	GetLocalTime(&st);
	if (st.wHour <= 2)
	{
		iDay--;
	}
	char buf[256] = { 0 };
	sprintf_s(buf, "%04u%02u%02u", iYear, iMon, iDay);
	QString fileName = "./data/";
	fileName += buf;
	fileName += ".json";
	return fileName;
}

bool QServer::checkAuth(QtExMsg * msg)
{

	try
	{
		QString data = msg->buf;
		auto date  = m_map[data.toUtf8()];
		if (date.isEmpty())
		{
			return false;
		}
		auto data1 =(UINT64) getstrDate().toLongLong();
		auto data2 = (UINT64)date.toLongLong();
		if (data2 >= data1)
		{
			qDebug() << "操作账户: " << data;
			return true;
		}
		else
		{
			return false;
		}

	}
	catch (int)
	{
		return false;
	}
	return false;
}

void QServer::getaccount()
{
	QFile file("./database.txt");
	file.open(QIODevice::ReadOnly);
	QString account = file.readAll();
	account.remove("\r\n");
	auto list = account.split(';');
	int len = list.size() - 1;
	for (int i = 0; i < len; i++)
	{
		auto  data = list[i];
		auto arr = data.split(',');
		m_map[arr[0]] = arr[1];
	}
}

long QServer::getres()
{
	SYSTEMTIME st, lt;
	GetLocalTime(&st);
	if (st.wHour < 3)
	{
		st.wHour += 24;
	}
	int num = 0;
	num = (st.wHour - 10) * 12;
	num += st.wMinute / 5;
	num++;
	return num;
}

QString QServer::getstrDate()
{
	auto date = QDate::currentDate();
	int iYear = date.year();
	int iMon = date.month();
	int iDay = date.day();
	char buf[256] = { 0 };
	sprintf_s(buf, "%04u%02u%02u", iYear, iMon, iDay);
	return QString(buf);
}

