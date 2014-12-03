#include <iostream>
#include "../Utils.h"
#include "../CEvent.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "../CThread.h"

using namespace std;
using namespace lce;

const int MAXLINE=4096;

CEvent oCEvent;

typedef struct
{
    char szBuf[MAXLINE+1];
    int iSize;
}SESession;


class CLFEvent :public CThread
{
public:
	static void onAccept(int iFd,void *pData)
	{
		cout<<"onAccept"<<endl;

		struct sockaddr_in stClientAddr;
		int iAddrLen = sizeof(struct sockaddr_in);

		int iClientSock=accept(iFd, (struct sockaddr *) &stClientAddr,(socklen_t *)&iAddrLen);
		if (!iClientSock)
		{
			cout<<"accept error"<<endl;
			return;
		}

		cout<<"iClientSock "<<iClientSock<<endl;
		lce::setNBlock(iClientSock);
		m_oCEvent.addFdEvent(iClientSock,CEvent::EV_READ,onRead,NULL);
	}
	static void onRead(int iFd,void *pData)
	{
		cout<<"onRead "<<iFd<<endl;

		SESession *pSession=new SESession();

		pSession->iSize=::read(iFd,pSession->szBuf,MAXLINE);

		if (pSession->iSize > 0)
		{
			pSession->szBuf[pSession->iSize]='\0';
			cout<<pSession->szBuf<<endl;
			m_oCEvent.addFdEvent(iFd,CEvent::EV_WRITE,CLFEvent::onWrite,pSession);
		}
		else if(pSession->iSize == 0)
		{
			delete pSession;
			m_oCEvent.delFdEvent(iFd,CEvent::EV_WRITE|CEvent::EV_READ);
			lce::close(iFd);
			cout<<"onClose "<<iFd<<endl;
		}
		else
		{
			delete pSession;
			m_oCEvent.delFdEvent(iFd,CEvent::EV_WRITE|CEvent::EV_READ);
			lce::close(iFd);
		}
	}

	static void onWrite(int iFd,void *pData)
	{
		cout<<"onWrite "<<iFd<<endl;
		SESession *pSession = (SESession*)pData;
		::send(iFd,pSession->szBuf,pSession->iSize,0);
		m_oCEvent.delFdEvent(iFd,CEvent::EV_WRITE);
		//::close(iFd);
		delete pSession;
	}

public:
	int run()
	{
		m_oCEvent.run();
	}

	bool init()
	{
		m_oCEvent.init();

	}

	CEvent & getEvent()
	{
		return m_oCEvent;
	}

private:

	CEvent m_oCEvent;
};

class CLFServer
{

public:
	CLFServer(const string &sIp,uint16_t wPort)
	{

	}

private:
	vector<CLFEvent> vecEvent;
};

void onWrite(int iFd,void *pData)
{
    cout<<"onWrite "<<iFd<<endl;
    SESession *pSession = (SESession*)pData;
    ::send(iFd,pSession->szBuf,pSession->iSize,0);
    oCEvent.delFdEvent(iFd,CEvent::EV_WRITE);
    //::close(iFd);
    delete pSession;
}

void onRead(int iFd,void *pData)
{
    cout<<"onRead "<<iFd<<endl;

    SESession *pSession=new SESession();

    pSession->iSize=::read(iFd,pSession->szBuf,MAXLINE);

    if (pSession->iSize > 0)
    {
        pSession->szBuf[pSession->iSize]='\0';
        cout<<pSession->szBuf<<endl;
        oCEvent.addFdEvent(iFd,CEvent::EV_WRITE,onWrite,pSession);
    }
    else if(pSession->iSize == 0)
    {
        delete pSession;
        oCEvent.delFdEvent(iFd,CEvent::EV_WRITE|CEvent::EV_READ);
        lce::close(iFd);
        cout<<"onClose "<<iFd<<endl;
    }
    else
    {
        delete pSession;
        oCEvent.delFdEvent(iFd,CEvent::EV_WRITE|CEvent::EV_READ);
        lce::close(iFd);
    }



}



void onAccept(int iFd,void *pData)
{
    cout<<"onAccept"<<endl;

    struct sockaddr_in stClientAddr;
    int iAddrLen = sizeof(struct sockaddr_in);

    int iClientSock=accept(iFd, (struct sockaddr *) &stClientAddr,(socklen_t *)&iAddrLen);
    if (!iClientSock)
    {
        cout<<"accept error"<<endl;
        return;
    }

    cout<<"iClientSock "<<iClientSock<<endl;
    lce::setNBlock(iClientSock);
    oCEvent.addFdEvent(iClientSock,CEvent::EV_READ,onRead,NULL);

}

void onConnect(int iFd,void *pData)
{

    oCEvent.delFdEvent(iFd,CEvent::EV_READ|CEvent::EV_WRITE);
    int err = 0;
    socklen_t errlen = sizeof(err);
    if(getsockopt(iFd, SOL_SOCKET, SO_ERROR, &err, &errlen) < 0)
    {
        cout<<"fail1"<<endl;
        return;
    }
    char szBuf[1024];
    int n=::recv(iFd,szBuf,1,0);

    if(n < 0 && errno == 11)
    {
        cout<<"onConnect"<<endl;
    }
    else if(n == 1)
    {
        cout<<"onConnect"<<endl;
    }
    else
    {
        cout<<"onConnectFail"<<endl;
    }
    cout<<"n="<<n<<endl;
    cout<<"errno="<<errno<<"msg="<<strerror(errno)<<endl;

    oCEvent.addFdEvent(iFd,CEvent::EV_READ,onRead,NULL);
    cout<<"onConnect"<<endl;

}



void onMessage(int iMsgType,void *pData)
{
    cout<<"onMessage"<<endl;
}

void onTimer(int iTimerId,void *pData)
{

    cout<<"onTimer"<<endl;
    oCEvent.addMessage(1,onMessage,NULL);
    oCEvent.addMessage(1,onMessage,NULL);
    oCEvent.addTimer(iTimerId,2000,onTimer,NULL);
/*
    int iFd=lce::createTcpSock();

    cout<<"fd="<<iFd<<endl;
    lce::setNBlock(iFd);

    int iRet=lce::connect(iFd,"127.0.0.1",80);

    if(iRet != -1)
    {
        cout<<"connect"<<endl;
    }
    else
    {
        if (errno == EINPROGRESS)
        {
            cout<<"add event"<<endl;
            oCEvent.addFdEvent(iFd,CEvent::EV_WRITE,onConnect,NULL);
        }
    }
*/
}



int main()
{
    int iSrvSock = 0;

    iSrvSock = lce::createTcpSock();

    if(iSrvSock < 0)
    {
        printf("create error=%d,msg=%s",errno,strerror(errno));
        return 0;
    }

	if(lce::bind(iSrvSock,"127.0.0.1",3030)< 0)
	{
		printf("bind error=%d,msg=%s",errno,strerror(errno));
		return 0;
	}

	if(lce::listen(iSrvSock) < 0)
	{
		printf("listen error=%d,msg=%s",errno,strerror(errno));
		return 0;
	}

    lce::setNCloseWait(iSrvSock);
    lce::setNBlock(iSrvSock);

	
	CLFEvent arEvent[2];

	arEvent[0].getEvent().addFdEvent(iSrvSock,CEvent::EV_READ,onAccept,NULL);

    oCEvent.addTimer(0,2000,onTimer,NULL);
    oCEvent.addFdEvent(iSrvSock,CEvent::EV_READ,onAccept,NULL);

    oCEvent.run();

    return 0;
}
 