#include <iostream>
#include "CAnyValue.h"
#include "CShm.h"
#include <stdlib.h>
#include "CShmArray.h"
#include "CBitMap.h"
#include "CHashMap.h"
using namespace std;
using namespace nce;

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

    CAnyValue oValue;
    oValue["abc"]=1;
    oValue["bcd"]="hello";
    oValue["aa"].push_back("aaa");
    oValue["aa"].push_back(0);

    oValue["xbv"]["xx"]=1;
    oValue["xbv"]["ms"]="xxxx";
    string sOutPut;
    oValue.encodeJSON(sOutPut,true);

    SData stData;
    stData.ab=1;
    stData.a='x';

    CShmArray<SData> oArray;
    oArray.init(0x11111,100);
    for(int i=0;i<90;i++)
    {
        stData.a=(char)(65+i);
        oArray[i]=stData;
    }

    for(int i=0;i<90;i++)
    {
        cout<<oArray[i].a;
    }

    CHashMap<SData> oHashMap;
    oHashMap.init(100);
    oHashMap.insert(100,stData);
    BITMAP obitmap;
    obitmap.init(0x1112,10000);

    return 0;
}

