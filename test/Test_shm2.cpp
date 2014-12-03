#include <iostream>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <queue>
#include <set>
#include <list>
#include <algorithm>
#include <string>
#include <fcntl.h>
#include "../CShmList.h"
#include "../CShmHashMap.h"
using namespace std;
using namespace lce;

#pragma pack(1)

struct SData
{
    int ab;
    char szBuf[20];
    char a;
};

#pragma pack()


int main(int argc,char *argv[])
{


	SData stData;
	stData.ab=123;
	stData.a='X';

	CShmList<SData> liData;
	liData.init(0x1111,4096,true);
	cout<<"maxsize ="<<liData.max_size()<<endl;


	for(int i=0;i<90;i++)
	{
		stData.ab = i;
		if (!liData.push_back(stData))
		{
			cout<<liData.getErrMsg()<<endl;
		}
	}


	liData.erase(liData.begin());

	liData.erase(liData.begin());

	liData.erase(liData.begin());

	liData.erase(liData.begin());

	//liData.clear();
	
	cout<<"size="<<liData.size()<<endl;

	
	int sizet = liData.size();


	CShmList<SData>::iterator it = liData.begin();
	for(;it != liData.end();++it)
	{
		cout<<it->ab<<endl;
	}
	cout<<"---------------------"<<endl;

	CShmList<SData>::reverse_iterator it2 = liData.rbegin();
	for(;it2 != liData.rend();++it2)
	{
		cout<<it2->ab<<endl;
	}


	/*

	for(int j=0;j<sizet;j++)
	{
		if(j%2)
		{
			cout<<liData.front()<<endl;
			liData.pop_front();
		}
		else
		{
			cout<<liData.back()<<endl;
			liData.pop_back();
		}

	}
	*/
    return 0;
}

