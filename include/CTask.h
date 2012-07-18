#ifndef __NCE_CTASK_H
#define __NCE_CTASK_H

#include <queue>
#include <vector>
#include "CThread.h"
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

using namespace std;

namespace lce
{

class CTask
{
public:
    typedef struct
    {
        int iTaskType;
        int iTimeOut;
        void *pData;
    }STaskInfo;

    class CTaskThread : public CThread
    {
    public:
        CTaskThread()
        {
            m_pTask=NULL;
        }

        ~CTaskThread()
        {

        }


        int init(CTask *pTask)
        {
            m_pTask=pTask;
            return 0;
        }

        int run()
        {
            while(!isStoped())
            {
                STaskInfo *pstTaskInfo = NULL;

                pthread_mutex_lock(&m_pTask->m_lock);

                if (!m_pTask->m_queTaskQueue.empty())
                {
                    pstTaskInfo=m_pTask->m_queTaskQueue.front();
                    m_pTask->m_queTaskQueue.pop();
                }
                else
                {
                    pthread_cond_wait(&m_pTask->m_cond, &m_pTask->m_lock);//线程睡眠等待
                }
                pthread_mutex_unlock(&m_pTask->m_lock);

                if (pstTaskInfo != NULL)
                {
                    m_pTask->onWork(pstTaskInfo->iTaskType,pstTaskInfo->pData);
                    delete pstTaskInfo;
                }

            }
            return 0;
        }
    private:
        CTask * m_pTask;
    };


    friend class CTaskThread;
    ~CTask();
    int init(int iThreadNum,int iTimeOut);
    int run();
    int stop();
    int dispatch(int iTaskType,void *pData);
    virtual void onWork(int iTaskType,void *pData) = 0;

    char *getErrMsg() { return m_szErrMsg;}
private:
    int m_iThreadNum;
    pthread_mutex_t  m_lock;
    pthread_cond_t m_cond;
    vector <CTaskThread *> m_vecTaskThreads;
    queue <STaskInfo *> m_queTaskQueue;
    char m_szErrMsg[1024];
    int m_iMaxSize;
};

};
#endif
