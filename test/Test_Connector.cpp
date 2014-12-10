#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "CTcpConnector.h"
#include "CUdpConnector.h"
#include <iostream>
#include <string>

using namespace std;
using namespace lce;

int main(int argc, char ** argv)
{

    if(argc < 4)
    {
        cout<<"usage:"<<argv[0]<<" ip port timeout"<<endl;
        return 0;
    }

    CTcpConnector oTcpConn;
    bool bRet = oTcpConn.connect(argv[1],atoi(argv[2]),atoi(argv[3]));

    if(!bRet)
    {
        cout<<"connect fail:"<<oTcpConn.getErrMsg()<<endl;
        return 0;
    }

    cout<<"connect ok" <<endl;

    string sReq="GET / HTTP/1.1\r\n";
    sReq.append("Host: ");
    sReq.append(argv[1]);
    sReq.append("\r\n");
    sReq.append("Connection: Close\r\n\r\n");
    oTcpConn.write(sReq.data(),sReq.size());

    string sResp;
    int iSize = oTcpConn.read(sResp);

    if(iSize < 0)
    {
        cout<<oTcpConn.getErrMsg()<<endl;
    }
    cout<<sResp<<endl;

    CUdpConnector oUdpConn;
    bRet = oUdpConn.connect("127.0.0.1",80);
    if(!bRet)
    {
        cout<<"connect fail:"<<oUdpConn.getErrMsg()<<endl;
        return 0;
    }


}
