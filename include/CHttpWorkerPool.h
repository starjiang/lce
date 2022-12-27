#ifndef CHTTPPROCESSOR_POOL_H
#define CHTTPPROCESSOR_POOL_H

#include "CCommMgr.h"
#include "CProcessor.h"
#include "CHttpWorker.h"
#include "CTask.h"


namespace lce
{

typedef void (*WORKER_POOL_ERROR_HANDLER)(const char *szErrMsg);

template<class T>
class CHttpWorkerPool : public CTask ,public CProcessor
{
public:
    CHttpWorkerPool();
    virtual ~CHttpWorkerPool();
    int init(uint32_t dwThreadNum = 32,uint32_t dwQueueSize = 10000);
    void setErrHandler(WORKER_POOL_ERROR_HANDLER pErrHandler = NULL)
    {
        m_pErrHandler = pErrHandler;
    }
public:
    void onRead(StSession& stSession, const char* pszData, const int iSize);
    void onConnect(StSession& stSession, bool bOk, void* pData){}
    void onClose(StSession& stSession){}
    void onError(StSession& stSession, const char* szErrMsg, int iError);
    void onMessage(int iMsgType, void* pData);
public:
    void onWork(int iTaskType,void *pData,int iIndex);

private:
    vector<CHttpWorker*> m_vecWorkers;
    WORKER_POOL_ERROR_HANDLER m_pErrHandler;
};

//------------------------------------------
template<class T>
CHttpWorkerPool<T>::~CHttpWorkerPool()
{
    for(size_t i=0;i<m_vecWorkers.size();i++)
    {
        delete m_vecWorkers[i];
    }
}
template<class T>
CHttpWorkerPool<T>::CHttpWorkerPool()
{

}

template<class T>
int CHttpWorkerPool<T>::init(uint32_t dwThreadNum,uint32_t dwQueueSize)
{
	for(size_t i=0;i<dwThreadNum;i++)
	{
		CHttpWorker * poWorker = new T;
		m_vecWorkers.push_back(poWorker);
	}

    if(CTask::init(dwThreadNum,dwQueueSize) < 0)
    {
        return -1;
    }
	return 0;
}

template<class T>
void CHttpWorkerPool<T>::onMessage(int iMsgType, void* pData)
{
    CHttpResponse *poResponse = (CHttpResponse*)pData;
    CCommMgr::getInstance().write(poResponse->getSession(),poResponse->getWriter().data(),poResponse->getWriter().size(),poResponse->getCloseFlag());
    delete poResponse;
    poResponse = NULL;
}

template<class T>
void CHttpWorkerPool<T>::onError(StSession& stSession, const char* szErrMsg, int iError)
{
    if(m_pErrHandler != NULL)  m_pErrHandler(szErrMsg);
}


template<class T>
void CHttpWorkerPool<T>::onRead(StSession& stSession, const char* pszData, const int iSize)
{
    CHttpRequest *poRequest = NULL;
    try
    {
        poRequest = new CHttpRequest;
        poRequest->setSession(stSession);
        poRequest->getReader().setData(pszData,iSize);

        if(dispatch(0,poRequest) < 0)
        {
            if(m_pErrHandler != NULL)  m_pErrHandler("task queue full");
        }
    }
    catch(const exception &e)
    {
        if(m_pErrHandler != NULL)  m_pErrHandler(e.what());

        if(poRequest != NULL) delete poRequest;

        poRequest = NULL;

        CCommMgr::getInstance().close(stSession);
    }
}

template<class T>
void CHttpWorkerPool<T>::onWork(int iTaskType,void *pData,int iIndex)
{
    CHttpRequest *poRequest = (CHttpRequest*)pData;
    CHttpResponse *poResponse = new CHttpResponse;
    poResponse->setSession(poRequest->getSession());
    try
    {
        m_vecWorkers[iIndex]->onRequest(*poRequest,*poResponse);
        if(CCommMgr::getInstance().sendMessage(iTaskType,this,poResponse)<0)
        {
            if(m_pErrHandler != NULL)  m_pErrHandler(CCommMgr::getInstance().getErrMsg());
        }
    }
    catch(const exception &e)
    {
        if(m_pErrHandler != NULL)  m_pErrHandler(e.what());

        delete poResponse;
        poResponse = NULL;
    }
    delete poRequest;
    poRequest = NULL;
}

};

#endif // CPROCESSOR_POOL_H
