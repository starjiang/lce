#include <iostream>
#include "Utils.h"
#include "CEvent.h"
#include "CCommMgr.h"
#include "CH2ShortT3PackageFilter.h"
#include "CRawPackageFilter.h"
#include "CPackage.h"
#include "CHttpPackageFilter.h"
#include "CHttpReader.h"
#include "CHttpWriter.h"
#include "CTask.h"
#include "CProcessor.h"
#include "CAnyValue.h"

using namespace std;
using namespace lce;

int iSrv1;

#pragma pack(1)

struct SHead
{
public:
	void setStx() { m_cStx = 0x2; }
	void setLen(uint32_t dwLen) { m_dwLen = htonl(dwLen); }
	void setCmd(uint16_t wCmd) { m_wCmd = htons(wCmd); }
	void setSeq(uint32_t dwSeq) { m_dwSeq = htonl(dwSeq); }
	uint32_t getLen() { return ntohl(m_dwLen); }
	uint16_t getCmd() { return ntohs(m_wCmd); }
	uint32_t getSeq() { return ntohl(m_dwSeq); }

private:
	uint8_t m_cStx;
	uint32_t m_dwLen;
	uint16_t m_wCmd;
	uint32_t m_dwSeq;
};

#pragma pack()

class CProCenter : public CTask, public CProcessor
{
private:
	CProCenter()
	{
		dwCount = 0;
		dwTimerCount = 0;
	}
	static CProCenter *m_pInstance;
	int dwCount;
	uint32_t dwTimerCount;

public:
	void onRead(StSession &stSession, const char *pszData, const int iSize)
	{
		cout << "pkg size=" << iSize << endl;
	}

	void onWork(int iTaskType, void *pData, int iIndex)
	{
	}

	void onMessage(int iMsgType, void *pData)
	{
	}

	void onClose(StSession &stSession)
	{
		printf("onclose id=%d\n", stSession.iFd);
	}

	void onConnect(StSession &stSession, bool bOk, void *pData)
	{
		if (bOk)
		{

			printf("onconnect id=%d ok\n", stSession.iFd);

			// CAnyValuePackage<SHead> oPkg;
			// oPkg["name"]="starjiang";
			// oPkg["pwd"]="840206";
			// oPkg["params"]["xxx"];

			// oPkg.head().setStx();
			// oPkg.head().setCmd(1001);
			// oPkg.encodeJSON();
			// oPkg.head().setLen(oPkg.size()+1);
			// oPkg.setEtx();

			// while(true)
			// {
			// 	CCommMgr::getInstance().write(stSession,oPkg.data(),oPkg.size(),false);
			// 	usleep(1);
			// }
		}
		else
		{
			printf("onconnect id=%d fail\n", stSession.iFd);
		}
	}

	void onError(StSession &stSession, const char *szErrMsg, int iError)
	{
		cout << szErrMsg << endl;
	}

	void onTimer(int iTimerId, void *pData)
	{

		int iFd = CCommMgr::getInstance().connect(iSrv1, "47.242.109.226", 55101);
	}

	void onSignal(int iSignal)
	{
		switch (iSignal)
		{
		case SIGINT:
		{
			cout << "stopping..." << endl;
			exit(0);
		}
		break;
		case SIGHUP:
		{
			cout << "sighup" << endl;
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

	CProCenter::getInstance().init(8, 50000);
	CProCenter::getInstance().run();

	if (CCommMgr::getInstance().init() < 0)
	{
		printf("%s\n", CCommMgr::getInstance().getErrMsg());
		return 0;
	}

	iSrv1 = CCommMgr::getInstance().createClient();

	if (iSrv1 < 0)
	{
		cout << CCommMgr::getInstance().getErrMsg() << endl;
	}

	CCommMgr::getInstance().setProcessor(iSrv1, &CProCenter::getInstance(), PKG_RAW);
	CCommMgr::getInstance().addTimer(0, 1000, &CProCenter::getInstance(), NULL);
	CCommMgr::getInstance().addSigHandler(SIGINT, &CProCenter::getInstance());

	CCommMgr::getInstance().start();
	return 0;
}
