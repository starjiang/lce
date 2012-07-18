#include <iostream>
#include "Utils.h"
#include "CEvent.h"
#include "CCommMgr.h"
#include "CH2ShortT3PackageFilter.h"
#include "CRawPackageFilter.h"
#include "CPackage.h"
#include "CHttpPackageFilter.h"
#include "CHttpParser.h"
#include "CHttpResponse.h"
#include "CTask.h"

using namespace std;
using namespace nce;

int iSrv1,iSrv2,iSrv3,iSrv4,iSrv5,iSrv6,iSrv7;

struct SRequest
{
    uint32_t dwReqId;
    SSession stSession;
    CHttpParser oParser;
    CHttpResponse oResponse;
};

class CProCenter : public CTask
{
private:
    CProCenter(){}
    static CProCenter *m_pInstance;
public:


    static bool onHttpRead(SSession &stSession,const char * pszData, const int iSize)
    {


        SRequest *pstRequest=new SRequest;

        pstRequest->stSession=stSession;

        pstRequest->oParser.setData(pszData,iSize);
        CProCenter::getInstance().dispatch(100,pstRequest);

    }


    void onWork(int iTaskType,void *pData)
    {

        SRequest *pstRequest=(SRequest*)pData;

        pstRequest->oResponse.begin();
        pstRequest->oResponse.setStatusCode(200);
        pstRequest->oResponse<<"Hello world";
        pstRequest->oResponse.end();

        CCommMgr::getInstance().sendMessage(iTaskType,CProCenter::onMessage,pstRequest);
    }

    static void onMessage(uint32_t dwMsgType,void *pData)
    {
        SRequest *pstRequest=(SRequest*)pData;
        CCommMgr::getInstance().write(pstRequest->stSession,pstRequest->oResponse.data(),pstRequest->oResponse.size(),true);
        delete pstRequest;
    }
    static CProCenter &getInstance()
    {
        if (NULL == m_pInstance)
        {
			m_pInstance = new CProCenter;
		}
		return *m_pInstance;
    }

};


CProCenter *CProCenter::m_pInstance = NULL;

#pragma pack(1)

struct SHead
{
public:
    void setStx(){ m_cStx = 0x2; }
    void setLen(uint16_t wLen){ m_wLen = htons(wLen); }
    void setSeq(uint32_t dwSeq){ m_dwSeq = htonl(dwSeq); }
    uint16_t getLen(){ return ntohs(m_wLen); }
    uint32_t getSeq(){ return ntohl(m_dwSeq); }

private:
    uint8_t m_cStx;
    uint16_t m_wLen;
    uint32_t m_dwSeq;
};

#pragma pack()


bool onRead(SSession &stSession,const char * pszData, const int iSize)
{

    if(stSession.iSvrId == iSrv1 || stSession.iSvrId == iSrv6 || stSession.iSvrId== iSrv5)
    {
        cout<<"srvid="<<stSession.iSvrId<<" len="<<iSize<<endl;
        string sMsg;

        CPackage<SHead> oPkg(pszData,iSize);

        oPkg.readString2(sMsg);
        cout<<"len="<<oPkg.head().getLen()<<"msg="<<sMsg<<endl;

        if(CCommMgr::getInstance().write(stSession,pszData,iSize,false)<0)
        {
            cout<<"error="<<CCommMgr::getInstance().getErrMsg()<<endl;
        }
    }
    else if(stSession.iSvrId == iSrv3)
    {
        cout<<"iSrv3 onRead:"<<pszData<<endl;
    }
    else if(stSession.iSvrId == iSrv4)
    {
        cout<<"iSrv4 onRead:"<<pszData<<endl;
        cout<<"iSrv4 write:"<<pszData<<endl;
        CCommMgr::getInstance().writeTo(iSrv4,"127.0.0.1",3004,pszData,iSize);
    }

}

void onClose(SSession &stSession)
{
    //cout<< "onClose"<<endl;
}
void onConnect(SSession &stSession,bool bOk)
{

    string sHello="helloworld";
    CPackage<SHead> oPkg;

    oPkg<<(uint16_t)sHello.size();
    oPkg<<sHello;
    oPkg<<(uint8_t)0x3;
    oPkg.head().setStx();
    oPkg.head().setLen(oPkg.size());

    cout<<"size="<<oPkg.size()<<endl;

    CCommMgr::getInstance().write(stSession,oPkg.data(),oPkg.size(),false);
    cout<< "onConnect"<<endl;
}
void onError(char * szErrMsg)
{
    cout<<szErrMsg<<endl;
}

void onTimer(uint32_t dwTimerId,void *pData)
{
    if(dwTimerId == 0)
    {
        cout<<"onTimer="<<dwTimerId<<endl;
        //CCommMgr::getInstance().writeTo(iSrv4,"127.0.0.1",3005,"123456",6);
        //CCommMgr::getInstance().addTimer(0,2000,onTimer,NULL);
    }
    else if(dwTimerId ==1)
    {
        //CCommMgr::getInstance().connect(iSrv6,"127.0.0.1",3000);
    }
    else if(dwTimerId ==2)
    {
        CCommMgr::getInstance().connect(iSrv6,"127.0.0.1",3000);
    }

}

void onSignal(int iSignal)
{
    switch(iSignal)
    {
        case SIGINT:
        {
            cout<<"stopping..."<<endl;
            CCommMgr::getInstance().stop();
            exit(0);
        }
        break;
        case SIGHUP:
        {
            cout<<"sighup"<<endl;
            exit(0);
        }
        break;
    }
}


int main()
{
    //CH2ShortT3PackageFilter oCPackageFilter;
    //nce::initDaemon(); //后台运行

    CProCenter::getInstance().init(8,2000);
    CProCenter::getInstance().run();

    if(CCommMgr::getInstance().init() < 0)
    {
        printf("%s\n",CCommMgr::getInstance().getErrMsg());
        return 0;
    }


    CH2ShortT3PackageFilter oCH2T3PackageFilter;
    CRawPackageFilter oCPackageFilter;
    CHttpPackageFilter oCHttpPackageFilter;

    iSrv1=CCommMgr::getInstance().createSrv(CCommMgr::SRV_TCP,"127.0.0.1",3000,1024*10,1024*100,1024*10,1024*100,&oCH2T3PackageFilter);
    CCommMgr::getInstance().setCallBack(iSrv1,onRead,onConnect,onClose,onError);


    iSrv2=CCommMgr::getInstance().createSrv(CCommMgr::SRV_TCP,"127.0.0.1",3002,1024*10,1024*100,1024*10,1024*100,&oCPackageFilter);
    CCommMgr::getInstance().setCallBack(iSrv2,onRead,onConnect,onClose,onError);

    iSrv3=CCommMgr::getInstance().createSrv(CCommMgr::SRV_UDP,"127.0.0.1",3004,1024*10,1024*100,1024*10,1024*100,&oCPackageFilter);
    CCommMgr::getInstance().setCallBack(iSrv3,onRead,onConnect,onClose,onError);

    iSrv4=CCommMgr::getInstance().createSrv(CCommMgr::SRV_UDP,"127.0.0.1",3005,1024*10,1024*100,1024*10,1024*100,&oCPackageFilter);
    CCommMgr::getInstance().setCallBack(iSrv4,onRead,onConnect,onClose,onError);


    iSrv5=CCommMgr::getInstance().createSrv(CCommMgr::SRV_TCP,"127.0.0.1",3001,1024*10,1024*100,1024*10,1024*100,&oCPackageFilter);
    CCommMgr::getInstance().setCallBack(iSrv5,onRead,onConnect,onClose,onError);


    iSrv6=CCommMgr::getInstance().createAsyncConn(CCommMgr::CONN_TCP,1024*10,1024*100,1024*10,1024*100,&oCPackageFilter);
    CCommMgr::getInstance().setCallBack(iSrv6,onRead,onConnect,onClose,onError);

    iSrv7=CCommMgr::getInstance().createSrv(CCommMgr::SRV_TCP,"0.0.0.0",8001,1024*10,1024*100,1024*10,1024*100,&oCHttpPackageFilter);

    if(iSrv7 < 0 )
    {
        cout<<CCommMgr::getInstance().getErrMsg()<<endl;
    }

    CCommMgr::getInstance().setCallBack(iSrv7,CProCenter::onHttpRead,onConnect,onClose,onError);


    CCommMgr::getInstance().addTimer(0,2000,onTimer,NULL);

    CCommMgr::getInstance().addTimer(1,2000,onTimer,NULL);

    CCommMgr::getInstance().addSigHandler(SIGINT,onSignal);

    CCommMgr::getInstance().start();
    return 0;
}
