#include <iostream>
#include <fstream>
#include "../include/CAnyValue.h"
#include "stdio.h"
#include <stdlib.h>
#include <sys/time.h>
using namespace std;
using namespace lce;


void tolong(const CAnyValue &oValue)
{
	oValue["aaw"].asUInt();
	cout<<"1----";
}

void tolong(unsigned int lValue)
{
	cout<<"2----";
	
}

int main(int argc,char *argv[])
{


	fstream ofile("conn_json.txt");
	ofile.seekg(0,ios::end);
	uint32_t dwFileSize = ofile.tellg();
	string sBuf8;
	sBuf8.resize(dwFileSize);
	ofile.seekg(0);
	ofile.read((char*)sBuf8.data(),dwFileSize);

	CAnyValue oValue4;
	string sBuf5;
	string sBuf6;

	timeval tv1;
	gettimeofday(&tv1, 0);
	uint64_t t1 = tv1.tv_sec * 1000000 + tv1.tv_usec;

	oValue4.decodeJSON(sBuf8.data(),sBuf8.size());

	timeval tv2;
	gettimeofday(&tv2, 0);
	uint64_t t2 = tv2.tv_sec * 1000000 + tv2.tv_usec;

	cout<<"span="<<t2-t1<<endl;

	gettimeofday(&tv1, 0);
	t1 = tv1.tv_sec * 1000000 + tv1.tv_usec;

	oValue4.encode(sBuf5);

	gettimeofday(&tv2, 0);
	t2 = tv2.tv_sec * 1000000 + tv2.tv_usec;

	cout<<"span2="<<t2-t1<<endl;


	gettimeofday(&tv1, 0);
	t1 = tv1.tv_sec * 1000000 + tv1.tv_usec;

	oValue4.encodeJSON(sBuf6);

	gettimeofday(&tv2, 0);
	t2 = tv2.tv_sec * 1000000 + tv2.tv_usec;

	cout<<"span5="<<t2-t1<<endl;

	
	CAnyValue oValue7;
	gettimeofday(&tv1, 0);
	t1 = tv1.tv_sec * 1000000 + tv1.tv_usec;

	
	oValue7.decode(sBuf5.data(),sBuf5.size());

	gettimeofday(&tv2, 0);
	t2 = tv2.tv_sec * 1000000 + tv2.tv_usec;

	cout<<"span3="<<t2-t1<<endl;

	//oValue4.encodeJSON(sBuf6);
	//cout<<"json" <<sBuf6<<endl;
	
	double flValue = -123455553234442444444444444444.111113334;
	char buffer[32];
	
	sprintf(buffer, "%#.16g", flValue);

	cout<<"flvalue="<<buffer<<endl;

	flValue = atof("-1.234555532344425E+29");

	cout<<flValue<<endl;

	uint64_t ddwInt = 0xffffffffffffffff;

	cout<<"ddwInt="<<ddwInt<<endl;
	
		

    CAnyValue oValue;
    oValue["abc"]=1;
    oValue["bcd"]="中国人helloaaa{}]\"'/\\*&^%$#@!><;:''''...,,中国人民大ddd";
	
	cout<<oValue["bcd"].asString()<<endl;

	oValue["aa"].push_back("aaa{}]\"'/\\*&^%$#@!><;:''''...,,");
    oValue["aa"].push_back(-1001);
	oValue["aa"].push_back(-123);
	oValue["aa"].push_back(-123);
	oValue["aa"].push_back(ddwInt);
    oValue["aa"].push_back(CAnyValue());


    oValue["xbv"]["xx"]=1.51111111;
	oValue["xbv"]["xx2"] = -123455553234442444444444444444.111113334;
    oValue["xbv"]["ms"]="xxxx";
    oValue["xbv"]["xxxx"] =true;
    oValue["xbv"]["x"];
	oValue["xbv"]["big"] = ddwInt;
	vector<string> vecKeys = oValue["xbv"].keys();
	cout <<"keysize="<< vecKeys.size()<<endl;
	
	for(int i=0;i<vecKeys.size();i++)
	{
		cout<<vecKeys[i]<<endl;
	}

	cout<<"is obj="<<oValue["xbv"].isObject()<<endl;

	tolong(oValue);
	ddwInt = oValue["xbv"]["big"].asUInt64();

	cout<<"ddwInt="<<ddwInt<<endl;

	string sData;
	string sData2;
	string sData3;
	string sData4;
    oValue.encodeJSON(sData);
	cout<<"json1="<<sData<<endl;
	oValue.clear();
	oValue.decodeJSON(sData.data(),sData.size());

	cout<< oValue["bcd"].asString()<<endl;

    oValue.encodeJSON(sData3);


	cout<<"json2="<<sData3<<endl;

	oValue.encode(sData2);
	oValue.clear();
	oValue.decode(sData2.data(),sData2.size());

	ddwInt = oValue["xbv"]["big"].asUInt64();
	cout<<"ddwInt="<<ddwInt<<endl;


	oValue.encodeJSON(sData4);
    cout<<"json3="<< sData4<<endl;

	oValue.clear();

	string str = "{\"a\":10}";

	oValue.decodeJSON(str.data(),str.size());


    return 0;
}
