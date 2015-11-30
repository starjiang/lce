#ifndef CANY_PROCESSOR_POOL_H
#define CANY_PROCESSOR_POOL_H

#include "CCommMgr.h"
#include "CProcessor.h"
#include "CAnyWorker.h"
#include "CTask.h"


namespace lce
{

typedef void (*WORKER_POOL_ERROR_HANDLER)(const char *szErrMsg);

template<class T>
class CAnyWorkerPool : public CTask ,public CProcessor
{
public:
    CAnyWorkerPool();
    virtual ~CAnyWorkerPool();
    int init(uint32_t dwThreadNum = 32,uint32_t dwQueueSize = 10000);
    void setErrHandler(WORKER_POOL_ERROR_HANDLER pErrHandler = NULL)
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
    vector<CAnyWorker*> m_vecWorkers;
    WORKER_POOL_ERROR_HANDLER m_pErrHandler;
};

//------------------------------------------
template<class T>
CAnyWorkerPool<T>::~CAnyWorkerPool()
{
    for(size_t i=0;i<m_vecWorkers.size();i++)
    {
        delete m_vecWorkers[i];
    }
}
template<class T>
CAnyWorkerPool<T>::CAnyWorkerPool()
{

}

template<class T>
int CAnyWorkerPool<T>::init(uint32_t dwThreadNum,uint32_t dwQueueSize)
{
	for(size_t i=0;i<dwThreadNum;i++)
	{
		CAnyWorker * poWorker = new T;
		m_vecWorkers.push_back(poWorker);
	}

    if(CTask::init(dwThreadNum,dwQueueSize) < 0)
    {
        return -1;
    }
	return 0;
}

template<class T>
void CAnyWorkerPool<T>::onMessage(int iMsgType, void* pData)
{
    CAnyResponse *poResponse = (CAnyResponse*)pData;
    CCommMgr::getInstance().write(poResponse->getSession(),poResponse->getWriter().data(),poResponse->getWriter().size(),poResponse->getCloseFlag());
    delete poResponse;
    poResponse = NULL;
}

template<class T>
void CAnyWorkerPool<T>::onError(SSession& stSession, const char* szErrMsg, int iError)
{
    if(m_pErrHandler != NULL)  m_pErrHandler(szErrMsg);
}


template<class T>
void CAnyWorkerPool<T>::onRead(SSession& stSession, const char* pszData, const int iSize)
{
    CAnyRequest *poRequest = NULL;
    try
    {
        poRequest = new CAnyRequest;
        poRequest->setSession(stSession);
        poRequest->getReader().decode(pszData,iSize);

        if(dispatch(0,poRequest) < 0)
        {
            if(m_pErrHandler != NULL)  m_pErrHandler("task queue full");
        }
    }
    catch(const exception &e)
    {
        if(m_pErrHandler != NULL)  m_pErrHandler(e.what());

        if(poRequest != NULL)    delete poRequest;

        CCommMgr::getInstance().close(stSession);

        poRequest = NULL;
    }
}

template<class T>
void CAnyWorkerPool<T>::onWork(int iTaskType,void *pData,int iIndex)
{
    CAnyRequest *poRequest = (CAnyRequest*)pData;
    CAnyResponse *poResponse = new CAnyResponse;
    poResponse->setSession(poRequest->getSession());
    try
    {
        m_vecWorkers[iIndex]->onRequest(*poRequest,*poResponse);
        poResponse->getWriter().encode();
	poResponse->getWriter().setEtx();
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
