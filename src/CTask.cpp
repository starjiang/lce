#include "CTask.h"

namespace lce
{
    int CTask::init(int iThreadNum,int iMaxSize)
    {
        m_iThreadNum=iThreadNum;
        m_iMaxSize=iMaxSize;
        pthread_mutex_init (&m_lock, 0);
        pthread_cond_init (&m_cond, 0);

        for(int i=0;i<m_iThreadNum;i++)
        {
            CTaskThread *pTaskThread=new CTaskThread;
            pTaskThread->init(this,i);
            m_vecTaskThreads.push_back(pTaskThread);
        }
        return 0;
    }

    int CTask::run()
    {
        for(int i=0;i<m_iThreadNum;i++)
        {
            m_vecTaskThreads[i]->start();
        }
        return 0;
    }

    int CTask::stop()
    {
        for(int i=0;i<m_iThreadNum;i++)
        {
            m_vecTaskThreads[i]->stop();
        }
        return 0;
    }

    int CTask::dispatch(int iTaskType,void *pData)
    {
        STaskInfo stTaskInfo;
        stTaskInfo.iTaskType = iTaskType;
        stTaskInfo.pData = pData;

        pthread_mutex_lock(&m_lock);

        if(m_queTaskQueue.size() > (size_t)m_iMaxSize ) //判定队列size 放在锁空间内
        {
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"file:%s,line:%d,queue over size :%d",__FILE__,__LINE__,(int)m_queTaskQueue.size());
			pthread_mutex_unlock(&m_lock);
            return -1;
        }
        m_queTaskQueue.push(stTaskInfo);

        pthread_mutex_unlock(&m_lock);
        pthread_cond_signal(&m_cond); //唤醒睡眠线程
        return 0;
    }

    CTask::~CTask()
    {
        pthread_mutex_destroy(&m_lock);
        pthread_cond_destroy(&m_cond);

        for(vector<CTaskThread *>::iterator it1=m_vecTaskThreads.begin();it1!=m_vecTaskThreads.end();++it1)
        {
            delete (*it1);
        }
    }
};
