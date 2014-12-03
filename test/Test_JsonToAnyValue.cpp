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

	if(argc<4)
	{
		cout<<"usage:"<<argv[0]<<" type infile outfile"<<endl;
		return 0;
	}

	int type = atoi(argv[1]);

	if(type == 1)
	{
		fstream oInFile(argv[2]);
		fstream oOutFile(argv[3],ios::out|ios::binary);

		oInFile.seekg(0,ios::end);
		uint32_t dwInFileSize = oInFile.tellg();

		string sInBuf;
		sInBuf.resize(dwInFileSize);
		oInFile.seekg(0);
		oInFile.read((char*)sInBuf.data(),dwInFileSize);

		CAnyValue oValue;

		oValue.decodeJSON(sInBuf.data(),sInBuf.size());

		string sOutBuf;

		oValue.encode(sOutBuf);
		cout<<sOutBuf.size()<<endl;
		oOutFile.write(sOutBuf.data(),sOutBuf.size());
		oInFile.close();
		oOutFile.close();
	}
	else 
	{
		fstream oInFile(argv[2]);
		fstream oOutFile(argv[3],ios::out|ios::binary);

		oInFile.seekg(0,ios::end);
		uint32_t dwInFileSize = oInFile.tellg();

		string sInBuf;
		sInBuf.resize(dwInFileSize);
		oInFile.seekg(0);
		oInFile.read((char*)sInBuf.data(),dwInFileSize);

		CAnyValue oValue;

		oValue.decode(sInBuf.data(),sInBuf.size());

		string sOutBuf;

		oValue.encodeJSON(sOutBuf);
		cout<<sOutBuf.size()<<endl;
		oOutFile.write(sOutBuf.data(),sOutBuf.size());
		oInFile.close();
		oOutFile.close();
	}




    return 0;
}
