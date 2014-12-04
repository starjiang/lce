#ifndef CPROCESSOR_POOL_H
#define CPROCESSOR_POOL_H

#include "CCommMgr.h"
#include "CProcessor.h"
#include "CWorker.h"
#include "CTask.h"


namespace lce
{

typedef void (*WORKER_POOL_ERROR_HANDLER)(const char *szErrMsg);

template<class T>
class CWorkerPool : public CTask ,public CProcessor
{
public:
    CWorkerPool();
    virtual ~CWorkerPool();
    int init(uint32_t dwThreadNum = 32,uint32_t dwQueueSize = 10000);
    int setErrHandler(WORKER_POOL_ERROR_HANDLER pErrHandler = NULL)
    {
        m_pErrHandler = pErrHandler;
    }
public:
    void onRead(SSession& stSession, const char* pszData, const int iSize);
    void onConnect(SSession& stSession, bool bOk, void* pData){}
    void onClose(SSession& stSession){}
    void onError(SSession& stSession, const char* szErrMsg, int iError);
    void onMessage(int iMsgType, void* pData);
public:
    void onWork(int iTaskType,void *pData,int iIndex);

private:
    vector<CWorker*> m_vecWorkers;
    WORKER_POOL_ERROR_HANDLER m_pErrHandler;
};

//------------------------------------------
template<class T>
CWorkerPool<T>::~CWorkerPool()
{
    for(int i=0;i<m_vecServers.size();i++)
    {
        delete m_vecServers[i];
    }
}
template<class T>
CWorkerPool<T>::CWorkerPool()
{

}

template<class T>
int CWorkerPool<T>::init(uint32_t dwThreadNum,uint32_t dwQueueSize)
{

	for(int i=0;i<dwThreadNum;i++)
	{
		CWorker * poWorker = new T;
		m_vecWorkers.push_back(poWorker);
	}

    if(CTask::init(dwThreadNum,dwQueueSize) < 0)
    {
        return -1;
    }
	return 0;
}

template<class T>
void CWorkerPool<T>::onMessage(int iMsgType, void* pData)
{
    SResponse *pstResponse = (SResponse*)pData;
    CCommMgr::getInstance().write(pstResponse->getSession(),pstResponse->getData().data(),pstResponse->getData().size(),false);
    delete pstResponse;
    pstResponse = NULL;
}

template<class T>
void CWorkerPool<T>::onError(SSession& stSession, const char* szErrMsg, int iError)
{
    if(m_pErrHandler != NULL)  m_pErrHandler(szErrMsg);
}


template<class T>
void CWorkerPool<T>::onRead(SSession& stSession, const char* pszData, const int iSize)
{
    SRequest *pstRequest = new SRequest;
    pstRequest->setSession(stSession);
    pstRequest->writeData(pszData,iSize);
    if(dispatch(0,pstRequest) < 0)
    {
        if(m_pErrHandler != NULL)  m_pErrHandler("task queue full");
    }
}

template<class T>
void CWorkerPool<T>::onWork(int iTaskType,void *pData,int iIndex)
{
    SRequest *pstRequest = (SRequest*)pData;
    SResponse *pstResponse = new SResponse;
    pstResponse->setSession(pstRequest->getSession());
    try
    {
        m_vecWorkers[iIndex]->onRequest(*pstRequest,*pstResponse);
        if(CCommMgr::getInstance().sendMessage(iTaskType,this,pstResponse)<0)
        {
            if(m_pErrHandler != NULL)  m_pErrHandler(CCommMgr::getInstance().getErrMsg());
        }
    }
    catch(const exception &e)
    {
        if(m_pErrHandler != NULL)  m_pErrHandler(e.what());

        delete pstResponse;
        pstResponse = NULL;
    }
    delete pstRequest;
    pstRequest = NULL;
}

};

#endif // CPROCESSOR_POOL_H
