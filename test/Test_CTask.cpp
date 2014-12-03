#include <iostream>
#include "../CTask.h"
#include <stdlib.h>
using namespace std;
using namespace lce;

class CDemoTask : public CTask
{

public:
    void onWork(int iTaskType,void *pData)
    {
        cout<<"onWork"<<endl;
    }

};

int main(int argc,char *argv[])
{

    CDemoTask *pDemoTask=new CDemoTask();
    pDemoTask->init(5,5000);
    pDemoTask->run();
    pDemoTask->dispatch(1,NULL);
    pDemoTask->dispatch(1,NULL);


    sleep(10);
    return 0;
}
