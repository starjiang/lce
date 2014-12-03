#ifndef __LCE_DEFINE_H__
#define __LCE_DEFINE_H__
#include "CProcessor.h"


namespace lce
{


struct SSession
{
	SSession(){		memset(this,0,sizeof(SSession));	}
	std::string getStrIp()	{	return inet_ntoa(stClientAddr.sin_addr);	}
	uint32_t getIp()	const {	return ntohl(stClientAddr.sin_addr.s_addr);	}
	uint16_t getPort()	{	return ntohs(stClientAddr.sin_port);	}
	int iSvrId;
	int iFd;
	void * pData;
	struct sockaddr_in stClientAddr;
	uint64_t ddwBeginTime;
	int64_t getDelayTime()
	{
		uint64_t ddwCurTime = lce::getTickCount();
		return ddwCurTime - ddwBeginTime;
	}
};

enum APP_TYPE
{
	SRV_TCP=1,
	SRV_UDP=2,
	CONN_TCP=3,
};

enum PKG_TYPE
{
	PKG_EXT = 0,
	PKG_H2ST3 = 1,
	PKG_H2LT3 = 2,
	PKG_HTTP = 3,
	PKG_RAW = 4,

};

enum ERR_TYPE
{
	ERR_SOCKET = 1,
	ERR_INVALID_PACKAGE = 2,
	ERR_MAX_CLIENT = 3,
	ERR_NO_BUFFER = 4,
	ERR_NOT_READY = 5,
	ERR_SYSTEM = 6,
	ERR_PKG_FILTER = 7,
	ERR_OP_EVENT = 8,

};

struct SServerInfo
{
	SServerInfo()
	{
		pProcessor=NULL;
		pPackageFilter=NULL;
		iPkgType = 0;
		iFd = 0;
		iSrvId = 0;
		iType = 0;
	}
	int iSrvId;
	int iFd;
	string sIp;
	uint16_t wPort;
	int iPkgType;
	CProcessor *pProcessor;
	uint32_t dwInitRecvBufLen;
	uint32_t dwMaxRecvBufLen;
	uint32_t dwInitSendBufLen;
	uint32_t dwMaxSendBufLen;
	CPackageFilter *pPackageFilter;
	int iType;

	~SServerInfo()
	{
		if(pPackageFilter != NULL && iPkgType!= PKG_EXT) { delete pPackageFilter;pPackageFilter = NULL ;}
	}
};

struct SClientInfo
{
	SClientInfo(){ memset(this,0,sizeof(SClientInfo));}
	int iSrvId;
	int iFd;
	bool bNeedClose;
	SServerInfo *pstServerInfo;
	CSocketBuf *pSocketRecvBuf;
	CSocketBuf *pSocketSendBuf;
	sockaddr_in stClientAddr;
	uint64_t ddwBeginTime;
	~SClientInfo()
	{
		if (pSocketRecvBuf != NULL) { delete pSocketRecvBuf;pSocketRecvBuf = NULL; }
		if (pSocketSendBuf != NULL) { delete pSocketSendBuf;pSocketSendBuf = NULL; }
	}
};
struct SProcessor
{
	CProcessor *pProcessor;
	void * pData;
};

};

#endif