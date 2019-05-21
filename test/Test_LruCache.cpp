#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <sstream>
#include <unistd.h>
#include "CLruCache.h"

using namespace std;
using namespace lce;

int main(int argc,char *argv[])
{
    CLruCache<string,string> cache(100);

    for(int i=0;i<102;i++)
    {
        if(i == 51)
        {
            cache.set("abc1","sddsdsdsds1");
        }
        cache.set("abc"+toStr(i),"daaddasdasdasdsdasdas"+toStr(i));
    }

    cache.clear();

    cache.set("abc1","sddsdsdsds1",5);

    sleep(1);

    /*
    for(int i=0;i<100;i++)
    {
        cache.del("abc"+toStr(i));
    }
    */

    string sValue;
    cache.get("abc1",sValue);
    cout<<"v="<<sValue<<endl;


    CLruCacheV2<string,string> cache2(100);

    for(int i=0;i<102;i++)
    {
        if(i == 51)
        {
            cache2.set("abc1","sddsdsdsds1");
        }
        cache2.set("abc"+toStr(i),"daaddasdasdasdsdasdas"+toStr(i));
    }

    //cache2.clear();


    cache2.set("abc1","sssssss2222",5);

    cout<<"size="<<cache2.getSize()<<endl;

    sleep(1);

    for(int i=0;i<102;++i)
    {
        string sValue2;
        cache2.get("abc"+toStr(i),sValue2);
        cout<<sValue2<<" ";
    }

    /*
    for(int i=0;i<100;i++)
    {
        cache.del("abc"+toStr(i));
    }
    */

    string sValue2;
    cache2.get("abc1",sValue2);
    cout<<"v="<<sValue2<<endl;

    return 0;
}


