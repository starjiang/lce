#include <iostream>
#include "../include/CBQueue.h"
#include <string>

using namespace std;
using namespace lce;

int main(int argc, char **argv)
{

	char *pBuffer = new char[1024*1024];
	CBQueue oSqueue;	
	oSqueue.create(pBuffer,1024*1024);

	try
	{
		while(true)
		{

			string sData = "dsddasasddsa";
			oSqueue.push(sData.data(),sData.size());
			oSqueue.push(sData.data(),sData.size());

		}

	}
	catch(const exception &e)
	{

		cout<<oSqueue.size()<<e.what();
	}

	cout<<oSqueue.size()<<endl;

	string sOutData;
	unsigned long dwLen;
	sOutData.resize(65535);
	oSqueue.pop((char*)sOutData.data(),dwLen);

	cout<<sOutData<<dwLen<<endl;
	cout<<oSqueue.size()<<endl;

	oSqueue.pop((char*)sOutData.data(),dwLen);

	cout<<sOutData<<dwLen<<endl;

	return 0;
}