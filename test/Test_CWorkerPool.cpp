#include <iostream>
#include "Utils.h"
#include "CEvent.h"
#include "CCommMgr.h"
#include "CRawWorkerPool.h"

using namespace std;
using namespace lce;

int iSrv1;

void onError(const char *szErrMsg)
{
    cout << "onError:" << szErrMsg << endl;
}

class CProWorker : public CRawWorker
{
public:
    void onRequest(CRawRequest &oRequest, CRawResponse &oResponse)
    {
        cout << oRequest.getReader() << endl;
        oResponse.getWriter().assign(oRequest.getReader());
    }
};

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        printf("usage:%s port\n", argv[0]);
        exit(0);
    }

    // lce::initDaemon(); //后台运行

    CRawWorkerPool<CProWorker> *poWorkerPool = new CRawWorkerPool<CProWorker>();
    poWorkerPool->init();
    poWorkerPool->setErrHandler(onError);
    poWorkerPool->run();

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

    CCommMgr::getInstance().setProcessor(iSrv1, poWorkerPool, PKG_RAW);
    CCommMgr::getInstance().start();

    return 0;
}
