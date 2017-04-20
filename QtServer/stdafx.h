#pragma once
// VC 2010 以后，要求源码设置 UTF-8 BOM  
#if defined(_MSC_VER) && (_MSC_VER >= 1600)  
# pragma execution_character_set("utf-8")  
#endif  



typedef struct _QtExMsg
{
	int head;
	int id;
	unsigned long long value;
	int hourseArry[10];
	char buf[500];

} QtExMsg;