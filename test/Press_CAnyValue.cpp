#include <iostream>
#include <fstream>
#include "../include/CAnyValue.h"
#include "stdio.h"
#include <stdlib.h>
#include <sys/time.h>
using namespace std;
using namespace lce;

int main(int argc,char *argv[])
{
	timeval tv1;
	gettimeofday(&tv1, 0);
	uint64_t t1 = tv1.tv_sec * 1000000 + tv1.tv_usec;

	string sData;

	for(int i=0;i<15000;i++)
	{

		CAnyValue oValue1;
		CAnyValue oValue2;
		oValue1["name"] = "dasssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss";
		oValue1["name1"] = "dasssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss";
		oValue1["name2"] = "dasssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss";
		oValue1["name3"] = "dasssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss";
		oValue1["name4"] = "dasssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss";
		oValue1["age"] = 123456;
		oValue1["num1"] = 1;
		oValue1["num2"] = 1;
		oValue1["num3"] = 1;
		oValue1["num4"] = 1;

		oValue2["ls"].push_back(oValue1);
		oValue2["ls"].push_back(oValue1);
		oValue2["ls"].push_back(oValue1);
		oValue2["ls"].push_back(oValue1);
		oValue2["data"] = oValue1;
		oValue2["data1"] = oValue1;
		oValue2["data2"] = oValue1;
		string sBuf;
		oValue2.encode(sBuf);
		if(i ==14999) 
			sData = sBuf;
	}

	timeval tv2;
	gettimeofday(&tv2, 0);
	uint64_t t2 = tv2.tv_sec * 1000000 + tv2.tv_usec;

	cout<<"span="<<t2-t1<<endl;


	for(int i=0;i<15000;i++)
	{
		CAnyValue oValue;
		oValue.decode(sData.data(),sData.size());
	}


	timeval tv3;
	gettimeofday(&tv3, 0);
	uint64_t t3= tv3.tv_sec * 1000000 + tv3.tv_usec;

	cout<<"span="<<t3-t2<<endl;

	CAnyValue oValue;
	oValue.push_back(123400000000000000);
	oValue.push_back(-1234);
	return 0;
}
