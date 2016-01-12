#include <iostream>
#include "CMysql.h"
#include "Utils.h"
#include <exception>

using namespace std;
using namespace lce;

int main(int argc,char **argv)
{
    CMySql oMysql;
    oMysql.Init("172.28.2.61","root","Xj$nbyndfwlb#C");
    oMysql.use("test");

    while(true)
    {
        try
        {
            cerr<<"query"<<endl;
            MySqlData result = oMysql.query("select * from user");
            cerr<<"query end"<<endl;
            for(int i=0;i<result.num_rows();i++)
            {
                cout<<"name="<<result[i]["name"]<<endl;
            }
            cout<<"======================"<<endl;
            sleep(3);
        }
        catch(const exception &e)
        {
            cout<<e.what()<<endl;
        }
    }
	return 0;
}
