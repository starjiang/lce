#include <iostream>
#include <CThread.h>

using namespace std;
using namespace Nce;

class CDemoThread public CThread
{

public:
/** @brief (one liner)
  *
  * (documentation goes here)
  */
int CThread::run()
{
    cout<<"dssssssssss"<<endl;
}

};

int main(int argc,char *argv[])
{

    CDemoThread oThread=new CDemoThread();
    oThread.start();

    return 0;
}
