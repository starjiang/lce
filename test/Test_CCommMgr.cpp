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
    CProCenter(){ dwCount = 0;dwOutCount=0;dwOutCount2=0;}
    static CProCenter *m_pInstance;
	int dwCount;
	int dwOutCount;
	int dwOutCount2;
public:


    void onRead(SSession &stSession,const char * pszData, const int iSize)
    {

		dwCount++;
        SRequest *pstRequest=new SRequest;

        pstRequest->stSession = stSession;
        /*
		pstRequest->oParser.setData(pszData,iSize);

		string sfile1 = pstRequest->oParser.getFile("file1");
		cout<<sfile1<<endl;

		string sfile2 = pstRequest->oParser.getFile("file2");
		cout<<sfile2<<endl;

		string sValue;
		pstRequest->oParser.getValue("submit",sValue);
		cout<<sValue<<endl;


		string sfilename1 = pstRequest->oParser.getFileName("file1");
		cout<<sfilename1<<endl;

		string sfilename2 = pstRequest->oParser.getFileName("file2");
		cout<<sfilename2<<endl;
		*/

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

		//usleep(2000);
		//cout<<"type="<<iTaskType<<endl;
        //cout<<"index="<< iIndex<<endl;
		SRequest *pstRequest=(SRequest*)pData;
        pstRequest->oResponse.begin();
        pstRequest->oResponse.setStatusCode(200);
		pstRequest->oResponse.setHead("Data","xxxxxxxxxx");
        pstRequest->oResponse<<"Hello world";
        pstRequest->oResponse.end();
        if(CCommMgr::getInstance().sendMessage(iTaskType,this,pstRequest)<0)
		{
			cout<<"end error"<<endl;
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
		dwCount++;
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
		cout<<dwTimerId<<" dwCount="<<dwCount<<" dwOut="<<dwOutCount<<endl;
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

CPackage<SHead> oPkg;



int main()
{
    //CH2ShortT3PackageFilter oCPackageFilter;
    //lce::initDaemon(); //后台运行

	int a;
	long b;
	oPkg<<(int)2;
	oPkg<<(uint64_t)3;
	oPkg>>a;
	oPkg>>b;


    CProCenter::getInstance().init(7,50000);
    CProCenter::getInstance().run();

    if(CCommMgr::getInstance().init(50000) < 0)
    {
        printf("%s\n",CCommMgr::getInstance().getErrMsg());
        return 0;
    }


    iSrv1=CCommMgr::getInstance().createSrv(SRV_TCP,"0.0.0.0",8001);

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
