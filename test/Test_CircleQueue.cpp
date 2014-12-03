#include<sys/time.h>
#include<stdint.h>
#include<iostream>
#include "../include/CCircleQueue.h"

using namespace std;
using namespace lce;

int main(int argc, char **argv)
{

	CCircleQueue<int> queue;

	queue.init(100);

	for(int i=0;i<100;i++)
		cout<<i<<" "<<queue.enque(i)<<endl;

	for(int i=0;i<100;i++)
	{
		int v = 0;
		queue.deque(v);
		cout<<v<<endl;
	}
    return 0;
}
