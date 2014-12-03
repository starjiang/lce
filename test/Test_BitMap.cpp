#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include "../CShm.h"
#include "../CBitMap.h"
using namespace std;
using namespace lce;


int main(int argc,char *argv[])
{

    BITMAP obitmap;
    obitmap.init(0x1112,1024*1024*100);//0x1112是共享内存KEY，开 1024*1024*100 个bit位
    obitmap.set(1001,true);//设置1001位，为true
    cout<<obitmap.get(1001)<<endl;//返回1001位 
    obitmap.set(1001,false);
    cout<<obitmap.get(1001)<<endl;
    //重置共享内存
    //obitmap.clear();

    return 0;
}

