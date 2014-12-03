#include <iostream>
#include <CNetWorkerMgr.h>
#include <CSingleton.h>
#include <CTask.h>

using namespace lce;
 

class CWorker:public CNetWorker,public CTask
{
public:
	int init()
	{
		m_dwCount = 0;
		addTimer(0,1000,NULL);
		m_iConnHandler = createAsyncConn(PKG_HTTP);
		cout<<m_iConnHandler<<endl;
		CTask::init(4,10000);
		CTask::run();
		return 0;
	}

	void onWork(int iTaskType,void *pData,int iIndex)
	{
		cout<<"onWork"<<endl;
		sendMessage(iTaskType,pData);
	}

	void onMessage(int iMsgType,void *pData)
	{
		cout<<"onMessage"<<endl;
	}

	void onRead(SSession &stSession,const char * pszData, const int iSize)
	{
		m_dwCount++;
		//write(stSession,pszData,iSize,true);

		
		if(stSession.iSvrId >= START_SRV_ID)
		{
			cout<<"id="<<getId()<<",fd="<<stSession.iFd<<",onRead"<<endl;

			string sRecv;
			sRecv.assign(pszData,iSize);
			cout<<sRecv;
		}
		else
		{
			string sData;
			sData+="HTTP/1.1 200 OK\r\n";
			sData+="Connection: Close\r\n";
			sData+="Content-Length: 5\r\n\r\n";  
			sData+="hello";
			//dispatch(0,NULL);
			write(stSession,(char*)sData.data(),sData.size());
		}


	}

	void onError(SSession &stSession,const char * szErrMsg,int iError)
	{
	
		cout<<"error:"<<szErrMsg<<endl;
	}

	void onClose(SSession &stSession)
	{
		cout<<"id="<<getId()<<",fd="<<stSession.iFd<<",onClose"<<endl;
	}
	void onConnect(SSession &stSession,bool bOk,void *pData)
	{
		//cout<<"id="<<getId()<<",fd="<<stSession.iFd<<",onConnect"<<endl;
		//string sReq = "GET / HTTP/1.1\r\n\r\n";
		//write(stSession,sReq.data(),sReq.size(),false);
	}    

	void onTimer(int iTimeId,void *pData)
	{
				
		cout<<"thread pid="<< getPid()<<endl;
		addTimer(0,1000,NULL);

		//connect(m_iConnHandler,"61.135.169.105",80);
		//addTimer(0,1000,NULL);

	}

private:
	int m_iConnHandler;
	uint32_t m_dwCount;
};

int main(int argc,char **argv)
{
 
	CNetWorkerMgr<CWorker> oNetWorkerMgr;
		
	if(oNetWorkerMgr.init(4,10000)< 0)
	{
		cout<<"init mgr error:"<<oNetWorkerMgr.getErrMsg()<<endl;
	}
	
	int iSrv1 = oNetWorkerMgr.createSrv("0.0.0.0",8001,PKG_HTTP);

	cout<<oNetWorkerMgr.getErrMsg()<<endl;

	oNetWorkerMgr.start();

	return 0;
}
