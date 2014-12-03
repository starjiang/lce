#include <iostream>
#include <CAsyncLog.h>

using namespace std;
using namespace lce;

int main(int argc,char **argv)
{
 
	CAsyncLog logAsync;

	logAsync.init("./test",1,1024*1024*10);

	for(int j=0;j<1000000;j++)
		logAsync.write("%d,%s",j,"abdfddsaaaaaaaaaaaadsasdadsasad1111111111111111222222222222222222222222222222222222222222222222222222222222222222222222111111111111111111111111111111111111111111111111111111");
	sleep(5);

	return 0;
}
