#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <sstream>
#include "../include/CLruCache.h"
using namespace std;
using namespace lce;


template <class T>
inline std::string toStr(const T &t)
{
    std::stringstream stream;
    stream<<t;
    return stream.str();
}


int main(int argc,char *argv[])
{
    CLruCache<string,string> cache;
    cache.init(100);

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

    sleep(10);

    /*
    for(int i=0;i<100;i++)
    {
        cache.del("abc"+toStr(i));
    }
    */

    string sValue;
    cache.get("abc1",sValue);
    cout<<"v="<<sValue<<endl;
    return 0;
}


