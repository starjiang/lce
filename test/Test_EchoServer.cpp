#include <iostream>
#include "Utils.h"
#include "CEvent.h"
#include "CCommMgr.h"
#include "CTask.h"
#include "CProcessor.h"

using namespace std;
using namespace lce;

int iSrv1;


class CProCenter : public CTask ,public CProcessor
{

private:

    CProCenter(){}

    static CProCenter *m_pInstance;

public:
	

    void onRead(SSession &stSession,const char * pszData, const int iSize)
    {

        CCommMgr::getInstance().write(stSession,pszData,iSize,false);
		
    }


    void onWork(int iTaskType,void *pData,int iIndex)
    {
 
    }



    void onMessage(int dwMsgType,void *pData)
    {

    }



	void onClose(SSession &stSession)
	{

	}

	void onConnect(SSession &stSession,bool bOk,void *pData)
	{

	}


	void onError(SSession &stSession,const char * szErrMsg,int iError)
	{
		cout<<szErrMsg<<endl;
	}

	void onTimer(int dwTimerId,void *pData)
	{
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

int main(int argc,char **argv)
{

	if(argc < 2)
	{
		printf("usage:%s port\n",argv[0]);
		exit(0);
	}

    lce::initDaemon(); //后台运行

    CProCenter::getInstance().init(4,50000);
    CProCenter::getInstance().run();

    if(CCommMgr::getInstance().init(50000) < 0)
    {
        printf("%s\n",CCommMgr::getInstance().getErrMsg());
        return 0;
    }


    iSrv1 = CCommMgr::getInstance().createSrv(SRV_TCP,"0.0.0.0",atoi(argv[1]));
	
    if(iSrv1 < 0 )
    {
        cout<<CCommMgr::getInstance().getErrMsg()<<endl;
		return 0;
    }

	CCommMgr::getInstance().setProcessor(iSrv1,&CProCenter::getInstance(),PKG_RAW);

    CCommMgr::getInstance().addSigHandler(SIGINT,&CProCenter::getInstance());

    CCommMgr::getInstance().start();

    return 0;
}
