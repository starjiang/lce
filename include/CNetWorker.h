#ifndef __NCE_NETWORKER_H
#define __NCE_NETWORKER_H

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <tr1/memory>
#include <tr1/functional>
#include <sys/eventfd.h>
#include "Utils.h"
#include "CSocketBuf.h"
#include "CPackageFilter.h"
#include "CH2T3PackageFilter.h"
#include "CH2ShortT3PackageFilter.h"
#include "CHttpPackageFilter.h"
#include "CRawPackageFilter.h"
#include "CEvent.h"
#include "CThread.h"
#include "CCircleQueue.h"
#include "Define.h"
using namespace std;

namespace lce
{
	static const float FD_TIMES =  1.2;
	static const uint32_t START_SRV_ID = 128;

	class CNetWorker :public CThread
	{
	public:

		CNetWorker()
		{
			m_iEventFd = 0;
		}

		virtual ~CNetWorker()
		{
			if(m_iEventFd > 0) ::close(m_iEventFd);

			for(size_t i=0;i<m_vecClients.size();i++)
			{
				if(m_vecClients[i] != NULL) 
				{
					delete m_vecClients[i];
				}
			}

			map<uint32_t,SServerInfo*>::iterator it = m_mapServers.begin();
			for(;it != m_mapServers.end();++it)
			{
				delete it->second;
			}

		}
		int init(uint32_t dwMaxClient);

		virtual int init(){ return 0;};

		virtual void onRead(SSession &stSession,const char * pszData, const int iSize){
			throw std::runtime_error("not implement onRead");
		}
		virtual void onClose(SSession &stSession){
			throw std::runtime_error("not implement onColse");
		}
		virtual void onConnect(SSession &stSession,bool bOk,void *pData){
			throw std::runtime_error("not implement onConnect");
		}
		virtual void onError(SSession &stSession,const char * szErrMsg,int iError){
			throw std::runtime_error("not implement  onError");
		}

		virtual void onTimer(int iTimeId,void *pData){ 
			throw std::runtime_error("not implement onTimer");
		}

		virtual void onMessage(int iMsgType,void *pData){ 
			throw std::runtime_error("not implement onMessage");
		}

		int watch(int iFd,void *pData);
		int close(const SSession &stSession);
		int write(const SSession &stSession,const char* pszData, const int iSize,bool bClose = true);

		int createAsyncConn(int iPkgType = PKG_RAW,uint32_t dwInitRecvBufLen =10240,uint32_t dwMaxRecvBufLen=102400,uint32_t dwInitSendBufLen=102400,uint32_t dwMaxSendBufLen=1024000);
		int setPkgFilter(int iSrvId,CPackageFilter *pPkgFilter);
		int connect(int iSrvId,const string &sIp,uint16_t wPort,void *pData = NULL);

		int addTimer(int iTimerId,uint32_t dwExpire,void *pData = NULL);
		int delTimer(int iTimerId);

		int sendMessage(int iMsgType,void* pData = NULL);

		const char * getErrMsg(){ return m_szErrMsg;}

	private:
		int run();

		inline bool isClose(int iFd)
		{
			return (m_vecClients[iFd] == NULL);
		}
		int close(int iFd);
		int write(int iFd);

		void onWrite(int iFd,void *pData);
		void onTcpRead(int iFd,void *pData);
		void onTcpConnect(int iFd,void *pData);
		void onEvent(int iFd,void *pData);

	protected:
		char m_szErrMsg[1024];
	private:
		CEvent m_oEvent;
		vector <SClientInfo *> m_vecClients;
		map <uint32_t,SServerInfo *> m_mapServers;
		CCircleQueue<SClientInfo*> m_queConn;
		int m_iEventFd;

	};
};

#endif