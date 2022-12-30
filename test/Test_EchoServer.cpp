#include <iostream>
#include "lce.h"
using namespace std;
using namespace lce;

int iSrv1;

class CProCenter : public CProcessor
{
private:
    CProCenter() {}
    static CProCenter *m_pInstance;

public:
    void onRead(StSession &stSession, const char *pszData, const int iSize)
    {
        CCommMgr::getInstance().write(stSession, pszData, iSize, false);
    }
    void onClose(StSession &stSession)
    {
    }
    void onConnect(StSession &stSession, bool bOk, void *pData)
    {
    }
    void onError(StSession &stSession, const char *szErrMsg, int iError)
    {
        cout << szErrMsg << endl;
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

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        printf("usage:%s port\n", argv[0]);
        exit(0);
    }

    // lce::initDaemon(); //后台运行

    if (CCommMgr::getInstance().init(50000) < 0)
    {
        printf("%s\n", CCommMgr::getInstance().getErrMsg());
        return 0;
    }

    iSrv1 = CCommMgr::getInstance().createSrv(SRV_TCP, "0.0.0.0", atoi(argv[1]));

    if (iSrv1 < 0)
    {
        cout << CCommMgr::getInstance().getErrMsg() << endl;
        return 0;
    }

    CCommMgr::getInstance().setProcessor(iSrv1, &CProCenter::getInstance(), PKG_RAW);
    CCommMgr::getInstance().start();

    return 0;
}
