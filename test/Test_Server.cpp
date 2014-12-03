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
#include "CProcessor.h"
#include "CAnyValue.h"

using namespace std;
using namespace lce;

int iSrv1;
uint32_t dwSeqNo = 0;

#pragma pack(1)

struct SHead
{
public:
	void setStx(){ m_cStx = 0x2; }
	void setLen(uint32_t dwLen){ m_dwLen = htonl(dwLen); }
	void setCmd(uint16_t wCmd){m_wCmd = htons(wCmd);}
	void setSeq(uint32_t dwSeq){ m_dwSeq = htonl(dwSeq); }
	uint32_t getLen(){ return ntohl(m_dwLen); }
	uint16_t getCmd(){ return ntohs(m_wCmd); }
	uint32_t getSeq(){ return ntohl(m_dwSeq); }

private:
	uint8_t m_cStx;
	uint32_t m_dwLen;
	uint16_t m_wCmd;
	uint32_t m_dwSeq;
};

#pragma pack()

class CProCenter : public CTask ,public CProcessor
{
private:
    CProCenter()
	{ 
		m_dwReqNum = 0;
	}

    static CProCenter *m_pInstance;

	uint32_t m_dwReqNum;

public:
	

    void onRead(SSession &stSession,const char * pszData, const int iSize)
    {
		m_dwReqNum++;
		CCommMgr::getInstance().write(stSession,pszData,iSize,false);
    }


    void onWork(int iTaskType,void *pData,int iIndex)
    {

    }



    void onMessage(uint32_t dwMsgType,void *pData)
    {

    }


	void onClose(SSession &stSession)
	{
		printf("onclose id=%d\n",stSession.iFd);
	}

	void onConnect(SSession &stSession,bool bOk,void *pData)
	{

	}

	void onError(SSession &stSession,const char * szErrMsg,int iError)
	{
		cout<<szErrMsg<<endl;
	}

	void onTimer(int iTimerId,void *pData)
	{
		if(iTimerId == 0)
		{
			CCommMgr::getInstance().addTimer(iTimerId,1000,this,pData);
			cout<<"dwReqNum="<<m_dwReqNum<<endl;
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

	CProCenter::getInstance().init(3,10000);
	CProCenter::getInstance().run();

	if(CCommMgr::getInstance().init(10000) < 0)
	{
		printf("%s\n",CCommMgr::getInstance().getErrMsg());
		return 0;
	}


	iSrv1=CCommMgr::getInstance().createSrv(CCommMgr::SRV_TCP,"0.0.0.0",8001);

	if(iSrv1 < 0 )
	{
		cout<<CCommMgr::getInstance().getErrMsg()<<endl;
	}

	CCommMgr::getInstance().setProcessor(iSrv1,&CProCenter::getInstance(),CCommMgr::PKG_H2LT3);
	CCommMgr::getInstance().addTimer(0,1000,&CProCenter::getInstance(),NULL);
    CCommMgr::getInstance().addSigHandler(SIGINT,&CProCenter::getInstance());

    CCommMgr::getInstance().start();
    return 0;
}
