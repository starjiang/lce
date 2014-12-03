#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include "../StringHelper.h"
using namespace std;
using namespace lce;


int main(int argc,char *argv[])
{

	string str = "		  xaxbcedaaaafffsssxa	  ";

	strReplace(str,"a","x");

	cout<<str<<endl;


	vector<string> vecStr;

	strSplit(str,"x",vecStr);

	for(size_t i=0;i<vecStr.size();i++)
	{
		cout<<vecStr[i]<<endl;
	}

	strTrim(str);

	cout<<str;

    return 0;
}

