#include "CThread.h"

namespace lce
{

CThread::CThread()
{

    m_iStop = 0 ;
}

CThread::~CThread()
{

}

int CThread::start()
{
    if (pthread_create(&m_iId,0,CThread::procThread,this)!=0)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"file:%s,line:%d,errno=%d,error=%s",__FILE__,__LINE__,errno,strerror(errno));
        return -1;
    }
    pthread_detach(m_iId);

    return 0;
}

int CThread::stop()
{
    m_iStop=1;
    return 0;
}

};
