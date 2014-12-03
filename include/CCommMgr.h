#ifndef __NCE_CCOMMGR_H
#define __NCE_CCOMMGR_H

#include <string>
#include <vector>
#include <iostream>
#include <tr1/memory>
#include <tr1/functional>
#include "Utils.h"
#include "CSocketBuf.h"
#include "CPackageFilter.h"
#include "CH2T3PackageFilter.h"
#include "CH2ShortT3PackageFilter.h"
#include "CHttpPackageFilter.h"
#include "CRawPackageFilter.h"
#include "CEvent.h"
#include "signal.h"
#include "CProcessor.h"
#include "Define.h"

using namespace std;
using namespace tr1;
namespace lce
{

static const float MAGIC_FD_TIMES =  1.2;

class CCommMgr
{

private:

	typedef tr1::unordered_map <int,SProcessor> MAP_TIMER_PROC;

public:
    int init(uint32_t dwMaxClient = 10000)
    {
        if(m_oCEvent.init(dwMaxClient * MAGIC_FD_TIMES)< 0)
        {
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
            return -1;
        }
		m_dwMaxClient = dwMaxClient;
		m_dwClientNum = 0;
        m_vecClients.resize(dwMaxClient * MAGIC_FD_TIMES);
        return 0;
    }

    int createSrv(int iType,const string &sIp,uint16_t wPort,uint32_t dwInitRecvBufLen =10240,uint32_t dwMaxRecvBufLen=102400,uint32_t dwInitSendBufLen=102400,uint32_t dwMaxSendBufLen=1024000);
    int createAsyncConn(uint32_t dwInitRecvBufLen =10240,uint32_t dwMaxRecvBufLen=102400,uint32_t dwInitSendBufLen=102400,uint32_t dwMaxSendBufLen=1024000);
    int setProcessor(int iSrvId,CProcessor * pProcessor,int iPkgType = PKG_RAW);
	int setPkgFilter(int iSrvId,CPackageFilter *pPkgFilter);

	int rmSrv(int iSrvId);


    int close(const SSession &stSession);
    int write(const SSession &stSession,const char* pszData, const int iSize,bool bClose = true);


    int writeTo(const int iSrvId, const string& sIp, const uint16_t wPort, const char* pszData, const int iSize);

    int connect(int iSrvId,const string &sIp,uint16_t wPort,void *pData = NULL);


    int addTimer(int iTimerId,uint32_t dwExpire,CProcessor * pProcessor,void *pData = NULL);
    int delTimer(int iTimerId);

    int addSigHandler(int iSignal,CProcessor * pProcessor);

    int start();
    int stop();
	

    int sendMessage(int dwMsgType,CProcessor * pProcessor,void* pData = NULL);

    const char * getErrMsg(){ return m_szErrMsg;}

    ~CCommMgr();

public:

    void onWrite(int iFd,void *pData);
    void onTcpRead(int iFd,void *pData);
    void onUdpRead(int iFd,void *pData);
    void onConnect(int iFd,void *pData);
    void onAccept(int iFd,void *pData);

	static void onSignal(int iSignal);

    static CCommMgr & getInstance()
    {
        if (NULL == m_pInstance)
        {
			m_pInstance = new CCommMgr;
		}
		return *m_pInstance;
    }
private:
    int close(int iFd);
    int write(int iFd);
    inline bool isClose(int iFd);

    CCommMgr(){}
	CCommMgr& operator=(const CCommMgr&);
	CCommMgr(const CCommMgr&);

private:
    vector <SServerInfo *> m_vecServers;
    vector <SClientInfo> m_vecClients;
	map<int,CProcessor*> m_mapSigProcs;
    CEvent m_oCEvent;
    char m_szErrMsg[1024];
    static CCommMgr *m_pInstance;
	uint32_t m_dwMaxClient;
	uint32_t m_dwClientNum;
};
};

#endif
