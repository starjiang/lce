#include <iostream>
#include "lce.h"


using namespace std;
using namespace lce;

int iSrv1;

struct SRequest
{
    uint32_t dwReqId;
    SSession stSession;
    CHttpParser oParser;
    CHttpResponse oResponse;

};

class CProCenter : public CTask ,public CProcessor
{
private:
    CProCenter()
    {
        dwInCount = 0;
        dwOutCount=0;
    }
    static CProCenter *m_pInstance;
    int dwInCount;
    int dwOutCount;
public:


    void onRead(SSession &stSession,const char * pszData, const int iSize)
    {

        dwInCount++;
        SRequest *pstRequest=new SRequest;

        pstRequest->stSession = stSession;

        /*
        pstRequest->oResponse.begin();
        pstRequest->oResponse.setStatusCode(200);
        pstRequest->oResponse<<"Hello world";
        pstRequest->oResponse.end();
        CCommMgr::getInstance().write(pstRequest->stSession,pstRequest->oResponse.data(),pstRequest->oResponse.size(),true);
        delete pstRequest;
        */

        //cout<<"onRead" <<endl;

        if(CProCenter::getInstance().dispatch(100,pstRequest)< 0)
        {
            cout<<CProCenter::getInstance().getErrMsg()<<endl;
        }

    }


    void onWork(int iTaskType,void *pData,int iIndex)
    {
        SRequest *pstRequest=(SRequest*)pData;
        pstRequest->oResponse.begin();
        pstRequest->oResponse.setStatusCode(200);
        pstRequest->oResponse.setHead("Data","xxxxxxxxxx");
        pstRequest->oResponse<<"Hello world";
        pstRequest->oResponse.end();
        if(CCommMgr::getInstance().sendMessage(iTaskType,this,pstRequest)<0)
        {
            cout<<"send message error"<<endl;
        }
    }



    void onMessage(int dwMsgType,void *pData)
    {
        //cout<<"onMessage"<<endl;
        dwOutCount++;
        SRequest *pstRequest=(SRequest*)pData;
        CCommMgr::getInstance().write(pstRequest->stSession,pstRequest->oResponse.data(),pstRequest->oResponse.size(),true);
        delete pstRequest;
    }


    void onClose(SSession &stSession)
    {
        //printf("onclose id=%d\n",stSession.iFd);
        //cout<< "onClose"<<endl;
        dwOutCount++;
    }

    void onConnect(SSession &stSession,bool bOk,void *pData)
    {
        //printf("onconnect id=%d\n",stSession.iFd);
    }

    void onError(SSession &stSession,const char * szErrMsg,int iError)
    {
        cout<<szErrMsg<<endl;
    }

    void onTimer(int dwTimerId,void *pData)
    {

        CCommMgr::getInstance().addTimer(dwTimerId,5000,this,pData);
        CCommMgr::getInstance().addTimer(3,2000,this,pData);
        CCommMgr::getInstance().addTimer(4,2000,this,pData);
        CCommMgr::getInstance().delTimer(1);
        CCommMgr::getInstance().delTimer(3);
        cout<<dwTimerId<<" dwInCount="<<dwInCount<<" dwOutCount="<<dwOutCount<<endl;
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
        }
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

int main()
{
    CProCenter::getInstance().init(7,50000);
    CProCenter::getInstance().run();

    if(CCommMgr::getInstance().init(50000) < 0)
    {
        printf("%s\n",CCommMgr::getInstance().getErrMsg());
        return 0;
    }

    iSrv1=CCommMgr::getInstance().createSrv(SRV_TCP,"0.0.0.0",8080);

    if(iSrv1 < 0 )
    {
        cout<<CCommMgr::getInstance().getErrMsg()<<endl;
    }

    CCommMgr::getInstance().setProcessor(iSrv1,&CProCenter::getInstance(),PKG_HTTP);

    CCommMgr::getInstance().addTimer(0,2000,&CProCenter::getInstance(),NULL);
    CCommMgr::getInstance().addTimer(1,5000,&CProCenter::getInstance(),NULL);
    CCommMgr::getInstance().addSigHandler(SIGINT,&CProCenter::getInstance());
    CCommMgr::getInstance().start();

    return 0;
}
