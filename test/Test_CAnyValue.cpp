#include <iostream>
#include "CAnyValue.h"
#include <stdlib.h>
using namespace std;
using namespace nce;

int jsonDecode(CAnyValue &oValue,const string &sSrc)
{
    int i = 0;
    while(i < sSrc.size())
    {
        if(sSrc[i] == '{')
        {

        }
        else if(sSrc[i] == '"')
        {

        }
        else if(sSrc[i] == '[')
        {

        }
        else if(sSrc[i] == ':' )
        {

        }
        else if(sSrc[i] == ',' )
        {

        }
    }

}



int main(int argc,char *argv[])
{

    CAnyValue oValue;
    oValue["abc"]=1;
    oValue["bcd"]="hello";
    oValue["aa"].push_back("aaa");
    oValue["aa"].push_back(0);

    oValue["xbv"]["xx"]=1.5;
    oValue["xbv"]["ms"]="xxxx";
    string sOutPut;
    oValue.encodeJSON(sOutPut,true);
    cout<< sOutPut<<endl;
    //{"aa":["aaa",0],"abc":1,"bcd":"hello","xbv":{"ms":"xxxx","xx":1}}
    string sInput="{\"aa\":[\"aaa\",0],\"abc\":1,\"bcd\":\"hello\",\"xbv\":{\"ms\":\"xxxx\",\"xx\":1}}";

    return 0;
}
