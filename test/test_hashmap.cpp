#include "CLruCache.h"
#include <iostream>
#include <string>
using namespace  std;
using namespace lce;

int main(int argc, char **argv)
{
	CLruCache<string,string> cache;
	cache.init();

	cache.set("dsdsds","dsasdasd");
	string sValue;
	cache.get("dsdsds",sValue);
	cout<<sValue<<endl;
}
