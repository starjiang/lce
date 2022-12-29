#ifndef __LCE_CTASK_H
#define __LCE_CTASK_H

#include <queue>
#include <vector>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "CThread.h"

using namespace std;

namespace lce
{

class CTask
{
public:
    typedef struct
    {
        int iTaskType;
        void *pData;
    }STaskInfo;

    class CTaskThread : public CThread
    {
    public:
        CTaskThread()
        {
            m_pTask=NULL;
			m_iIndex = 0;
        }

        ~CTaskThread()
        {

        }


        int init(CTask *pTask,int iIndex)
        {
            m_pTask=pTask;
			m_iIndex = iIndex;
            return 0;
        }

        int run()
        {
            while(!isStoped())
            {
                STaskInfo stTaskInfo;
				bool bHaveTask = false;

                pthread_mutex_lock(&m_pTask->m_lock);

                if(m_pTask->m_queTaskQueue.empty())
                {
                    pthread_cond_wait(&m_pTask->m_cond, &m_pTask->m_lock);//线程睡眠等待
                }
				else
				{
					stTaskInfo = m_pTask->m_queTaskQueue.front();
					m_pTask->m_queTaskQueue.pop();
					bHaveTask = true;
				}

                pthread_mutex_unlock(&m_pTask->m_lock);

                if (bHaveTask)
                {
                    m_pTask->onWork(stTaskInfo.iTaskType,stTaskInfo.pData,m_iIndex);
                }

            }
            return 0;
        }
    private:
        CTask * m_pTask;
		int m_iIndex;
    };


    friend class CTaskThread;
    virtual ~CTask();
    int init(int iThreadNum,int iMaxSize);
    int run();
    int stop();
    int dispatch(int iTaskType,void *pData);
    virtual void onWork(int iTaskType,void *pData,int iIndex) = 0;

    char *getErrMsg() { return m_szErrMsg;}
private:
    int m_iThreadNum;
    pthread_mutex_t  m_lock;
    pthread_cond_t m_cond;
    vector <CTaskThread *> m_vecTaskThreads;
    queue <STaskInfo> m_queTaskQueue;
    char m_szErrMsg[1024];
    int m_iMaxSize;
};

};
#endif
