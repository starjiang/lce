#include<iostream>
#include "CBlockQueue.h"
#include <stdlib.h>
#include <unistd.h>
using namespace std;
using namespace lce;

CBlockQueue<int> g_oQueue(50);

void *p(void *args)
{
	int data = 0;

	for(int i = 0; i < 120; i++)
	{
		cout<<"push item "<<data<<endl;
		g_oQueue.push(data++);
	}
	return NULL;
}

void *c(void* args)
{
	while(true)
	{
		int t = 0;
		g_oQueue.pop(t);
		sleep(1);
		cout<<"block="<<t<<endl;

	}

	return NULL;
}

int main()
{
	pthread_t id;
	pthread_create(&id, NULL, p, NULL);
	//pthread_create(&id, NULL, p, NULL);
	//pthread_create(&id, NULL, c, NULL);
	pthread_create(&id, NULL, c, NULL);
	for(;;)sleep(1);
	return 0;
}
