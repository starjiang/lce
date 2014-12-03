#include <iostream>
#include "../CThread.h"
#include <stdlib.h>
using namespace std;
using namespace lce;

class CDemoThread : public CThread
{

public:

    int run()
    {
        cout<<"dssssssssss"<<endl;
    }

};

int main(int argc,char *argv[])
{

    CDemoThread *oThread=new CDemoThread();
    oThread->start();
    sleep(10);
    return 0;
}
