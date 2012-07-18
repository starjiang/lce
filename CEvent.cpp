#include "CEvent.h"

using namespace std;
namespace lce
{

CEvent::CEvent()
{
    ::pthread_mutex_init(&m_lock,0);
    m_bRun=true;
    m_stFdEvents=NULL;
    m_stEPollState.iEPollFd= -1;
    memset(m_szTimeEventIndexs,0,sizeof(m_szTimeEventIndexs));
    m_iMsgFd[0]=-1;
    m_iMsgFd[1]=-1;

}

CEvent::~CEvent()
{
    ::pthread_mutex_destroy(&m_lock);

    if(m_iMsgFd[0] >=0 )  ::close(m_iMsgFd[0]);
    if(m_iMsgFd[1] >=0 )  ::close(m_iMsgFd[1]);


    if(m_stFdEvents != NULL)
    {
        delete  []m_stFdEvents;
        m_stFdEvents = NULL;
    }


    if (m_stEPollState.iEPollFd >= 0)
    {
        ::close(m_stEPollState.iEPollFd);
    }

    for(multiset<STimeEvent*>::iterator it=m_setSTimeEvents.begin();it!=m_setSTimeEvents.end();++it)
    {
        delete (*it);
    }

    while(!m_queMsgEvents.empty())
    {
        SMsgEvent *pstMsgEvent=m_queMsgEvents.front();
        delete pstMsgEvent;
        m_queMsgEvents.pop();
    }


}

int CEvent::init()
{

    m_stEPollState.iEPollFd = epoll_create ( EPOLL_MAX_SIZE );

    if( m_stEPollState.iEPollFd < 0 )
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d CEvent::init epoll_create error",__FILE__,__LINE__);
        return -1;
    }

    m_stFdEvents =new SFdEvent[EPOLL_MAX_SIZE];

    if(m_stFdEvents == NULL)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d CEvent::init memory limit",__FILE__,__LINE__);
        return -1;
    }


    m_dwTimerNum =0;

    for(int i=0;i<EPOLL_MAX_SIZE;i++)
    {
        m_stFdEvents[i].iEventType=EV_DONE;
    }

    if( pipe(m_iMsgFd) != 0)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d CEvent::init create pipe error",__FILE__,__LINE__);
        return -1;
    }

    //set nonblock
    int iOpts;
    iOpts=fcntl(m_iMsgFd[0],F_GETFL);
    if(iOpts<0)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d CEvent::init fcntl error",__FILE__,__LINE__);
        return -1;
    }
    iOpts = iOpts|O_NONBLOCK;

    if(fcntl(m_iMsgFd[0],F_SETFL,iOpts)<0)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d CEvent::init fcntl error",__FILE__,__LINE__);
        return -1;
    }

    addFdEvent(m_iMsgFd[0],CEvent::EV_READ,NULL,NULL);//自定义事件通知

    return 0;
}


int CEvent::addFdEvent ( int iWatchFd, int iEventType, fdEventCb pFdCb,void * pClientData=NULL)
{

    if (iWatchFd>= EPOLL_MAX_SIZE)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d CEvent::addFdEvent too big iWathchFd error",__FILE__,__LINE__);
        return -1;
    }


    struct epoll_event stEvent;

    stEvent.events=0;


    int iOP=m_stFdEvents[iWatchFd].iEventType == EV_DONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    m_stFdEvents[iWatchFd].iEventType = m_stFdEvents[iWatchFd].iEventType | iEventType;

    if (m_stFdEvents[iWatchFd].iEventType & EV_READ) stEvent.events |= EPOLLIN;
    if (m_stFdEvents[iWatchFd].iEventType & EV_WRITE) stEvent.events |= EPOLLOUT;

    stEvent.data.u64 = 0; /* avoid valgrind warning */
    stEvent.data.fd = iWatchFd;

    m_stFdEvents[iWatchFd].pClientData=pClientData;

    if(iEventType & EV_READ)    m_stFdEvents[iWatchFd].pReadProc=pFdCb;
    if(iEventType & EV_WRITE)     m_stFdEvents[iWatchFd].pWriteProc=pFdCb;

    if(epoll_ctl(m_stEPollState.iEPollFd,iOP,iWatchFd,&stEvent)== -1)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d CEvent::addFdEvent epoll_ctl error",__FILE__,__LINE__);
        return -1;
    }


    return 0;

}

/** @brief (one liner)
  *
  * (documentation goes here)
  */
int CEvent::delFdEvent(int iWatchFd, int iEventType)
{
    if (iWatchFd>= EPOLL_MAX_SIZE)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d CEvent::delFdEvent too big iWathchFd error",__FILE__,__LINE__);
        return -1;
    }

    struct epoll_event stEvent;
    stEvent.events=0;

    m_stFdEvents[iWatchFd].iEventType = m_stFdEvents[iWatchFd].iEventType & (~iEventType);

    if (m_stFdEvents[iWatchFd].iEventType & EV_READ) stEvent.events |= EPOLLIN;
    if (m_stFdEvents[iWatchFd].iEventType & EV_WRITE) stEvent.events |= EPOLLOUT;

    stEvent.data.u64 = 0; /* avoid valgrind warning */
    stEvent.data.fd = iWatchFd;

    if (m_stFdEvents[iWatchFd].iEventType != EV_DONE)
    {
        if (epoll_ctl(m_stEPollState.iEPollFd,EPOLL_CTL_MOD,iWatchFd,&stEvent)== -1)
        {
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d CEvent::delFdEvent epoll_ctl error",__FILE__,__LINE__);
            return -1;
        }
    }
    else
    {

        if(epoll_ctl(m_stEPollState.iEPollFd,EPOLL_CTL_DEL,iWatchFd,&stEvent) == -1)
        {
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d CEvent::delFdEvent epoll_ctl error",__FILE__,__LINE__);
            return -1;
        }
    }

    return 0;
}

/** @brief (one liner)
  *
  * (documentation goes here)
  */
int CEvent::delTimer(uint32_t dwTimerId)
{
    if(dwTimerId >=(uint32_t) CEVENT_MAX_TIME_EVENT)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d CEvent::delTimer too big dwTimerId error",__FILE__,__LINE__);
        return -1;
    }

    if(m_szTimeEventIndexs[dwTimerId] == 0)
    {
        return 0;
    }

    STimeEvent *pstTimeEvent =new STimeEvent;

    if(pstTimeEvent == NULL)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d no momery error",__FILE__,__LINE__);
        return -1;
    }

    pstTimeEvent->ddwMillSecs=m_szTimeEventIndexs[dwTimerId];

    multiset<STimeEvent *>::iterator find = m_setSTimeEvents.find(pstTimeEvent);

    for(;find!=m_setSTimeEvents.end();++find)
    {

        if((*find)->dwTimerId == dwTimerId)
        {
            delete (*find);
            m_setSTimeEvents.erase(find);
            break;
        }
    }
    delete pstTimeEvent;
    m_szTimeEventIndexs[dwTimerId]=0;

    return 0;

}


/** @brief (one liner)
  *
  * (documentation goes here)
  */
int CEvent::addTimer(uint32_t dwTimerId,uint64_t ddwExpire, timeEventCb pTimeCb, void* pClientData)
{

    if(dwTimerId >= (uint32_t)CEVENT_MAX_TIME_EVENT)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d CEvent::addTimer too big dwTimerId error",__FILE__,__LINE__);
        return -1;
    }

    if(m_szTimeEventIndexs[dwTimerId]!=0)
    {
        delTimer(dwTimerId);
    }

    STimeEvent *pstTimeEvent =new STimeEvent;
    if(pstTimeEvent == NULL)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d no momery error",__FILE__,__LINE__);
        delete pstTimeEvent;
        return -1;
    }
    pstTimeEvent->ddwMillSecs=CEvent::getMillSecsNow()+ddwExpire;
    pstTimeEvent->dwTimerId=dwTimerId;
    pstTimeEvent->pClientData=pClientData;
    pstTimeEvent->pTimeProc=pTimeCb;

    m_szTimeEventIndexs[dwTimerId]=pstTimeEvent->ddwMillSecs;

    m_setSTimeEvents.insert(pstTimeEvent);
    return 0;
}


int CEvent::addMessage(uint32_t dwMsgType,msgEventCb pMsgCb,void * pClientData)
{
    SMsgEvent *pstMsgEvent = new SMsgEvent;
    pstMsgEvent->dwMsgType=dwMsgType;
    pstMsgEvent->pMsgProc=pMsgCb;
    pstMsgEvent->pClientData=pClientData;

    ::pthread_mutex_lock(&m_lock);
    m_queMsgEvents.push(pstMsgEvent);
    ::pthread_mutex_unlock(&m_lock);

    if(write(m_iMsgFd[1],"1",1) < 0)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d CEvent::addMessage write error",__FILE__,__LINE__);
        delete pstMsgEvent;
        return -1;
    }

    return 0;
}


int CEvent::run()
{
    m_bRun = true;

    while(m_bRun)
    {
        uint64_t ddwTimeNow=CEvent::getMillSecsNow();
        uint64_t ddwExpire = EPOLL_WAIT_TIMEOUT;

        for(multiset<STimeEvent*>::iterator it=m_setSTimeEvents.begin();it!=m_setSTimeEvents.end();)
        {
            if ((*it)->ddwMillSecs<= ddwTimeNow)
            {
                STimeEvent * pstTimeEvent=(*it);
                m_szTimeEventIndexs[(*it)->dwTimerId] = 0;
                m_setSTimeEvents.erase(it++);

                if( pstTimeEvent->pTimeProc )
                    pstTimeEvent->pTimeProc(pstTimeEvent->dwTimerId,pstTimeEvent->pClientData);
                delete pstTimeEvent;


            }
            else
            {
                multiset<STimeEvent*>::iterator it=m_setSTimeEvents.begin();
                if(it!=m_setSTimeEvents.end() && ((*it)->ddwMillSecs-ddwTimeNow) > 0 )
                    ddwExpire=(*it)->ddwMillSecs-ddwTimeNow;
                break;
            }

        }

        int iEventNum=epoll_wait(m_stEPollState.iEPollFd,m_stEPollState.stEvents,EPOLL_MAX_EVENT,ddwExpire);

        for(int i=0;i<iEventNum;i++)
        {
            if((m_stEPollState.stEvents[i].events & EPOLLIN) ||
               (m_stEPollState.stEvents[i].events & EPOLLERR) ||
               (m_stEPollState.stEvents[i].events & EPOLLHUP))
            {
                if (m_stEPollState.stEvents[i].data.fd == m_iMsgFd[0]) //自定义事件
                {
                    char szBuf[1];
                    read(m_iMsgFd[0],szBuf,1);
                    SMsgEvent *pstMsgEvent = NULL;

                    ::pthread_mutex_lock(&m_lock);
                    if (!m_queMsgEvents.empty())
                    {
                        pstMsgEvent = m_queMsgEvents.front();
                        m_queMsgEvents.pop();
                    }
                    ::pthread_mutex_unlock(&m_lock);

                    if(pstMsgEvent != NULL)
                    {
                        if (pstMsgEvent->pMsgProc)
                            pstMsgEvent->pMsgProc(pstMsgEvent->dwMsgType,pstMsgEvent->pClientData);
                        delete pstMsgEvent;
                    }

                }
                else
                {
                    SFdEvent &stFdEvent=m_stFdEvents[m_stEPollState.stEvents[i].data.fd];
                    if(stFdEvent.pReadProc)
                        stFdEvent.pReadProc(m_stEPollState.stEvents[i].data.fd,stFdEvent.pClientData);
                }
            }
            if(m_stEPollState.stEvents[i].events & EPOLLOUT)
            {

                SFdEvent &stFdEvent=m_stFdEvents[m_stEPollState.stEvents[i].data.fd];
                if(stFdEvent.pWriteProc)
                    stFdEvent.pWriteProc(m_stEPollState.stEvents[i].data.fd,stFdEvent.pClientData);
            }
        }

    }
    return 0;
}

int CEvent::stop()
{
    m_bRun=false;
    return 0;
}

};
