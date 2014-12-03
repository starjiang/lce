#include "CEvent.h"

using namespace std;
namespace lce
{

CEvent::CEvent()
{
    ::pthread_mutex_init(&m_lock,0);
    m_bRun=false;
	m_bInit = false;
    m_stFdEvents = NULL;
    m_stEPollState.iEPollFd= -1;
    m_iMsgFd[0]=-1;
    m_iMsgFd[1]=-1;
	m_dwMaxFdNum = 0;

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

}

int CEvent::init(uint32_t dwMaxFdNum /* = 10000 */)
{
	if(m_bInit)
	{
		return 0;
	}
		

	m_dwMaxFdNum = dwMaxFdNum;

    m_stEPollState.iEPollFd = epoll_create ( dwMaxFdNum );

    if( m_stEPollState.iEPollFd < 0 )
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d CEvent::init epoll_create error",__FILE__,__LINE__);
        return -1;
    }

    m_stFdEvents =new SFdEvent[dwMaxFdNum];

    for(int i=0;i< (int)dwMaxFdNum;i++)
    {
        m_stFdEvents[i].iEventType=EV_DONE;
        m_stFdEvents[i].pClientRData = NULL;
        m_stFdEvents[i].pClientWData = NULL;
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

	m_bInit = true;
    return 0;
}


int CEvent::addFdEvent ( int iWatchFd, int iEventType, const HandlerEvent &onEvent,void * pClientData=NULL)
{

    if (iWatchFd >= (int)m_dwMaxFdNum || iWatchFd < 0)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d CEvent::addFdEvent too big iWathchFd error",__FILE__,__LINE__);
        return -1;
    }

    struct epoll_event stEvent;

    stEvent.events = 0;


    int iOP=m_stFdEvents[iWatchFd].iEventType == EV_DONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    m_stFdEvents[iWatchFd].iEventType = m_stFdEvents[iWatchFd].iEventType | iEventType;

    if (m_stFdEvents[iWatchFd].iEventType & EV_READ) stEvent.events |= EPOLLIN;
    if (m_stFdEvents[iWatchFd].iEventType & EV_WRITE) stEvent.events |= EPOLLOUT;

    stEvent.data.u64 = 0; /* avoid valgrind warning */
    stEvent.data.fd = iWatchFd;

    if(iEventType & EV_READ)
    {
        m_stFdEvents[iWatchFd].onRead = onEvent;
        m_stFdEvents[iWatchFd].pClientRData = pClientData;
    }

    if(iEventType & EV_WRITE)
    {
        m_stFdEvents[iWatchFd].onWrite = onEvent;
        m_stFdEvents[iWatchFd].pClientWData = pClientData;
    }

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
    if (iWatchFd >= (int)m_dwMaxFdNum || iWatchFd < 0)
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
int CEvent::delTimer(int iTimerId)
{

	MAP_TIME_INDEX::iterator it = m_mapTimeEventIndexs.find(iTimerId);

	if(it == m_mapTimeEventIndexs.end())
	{
		return 0;
	}

    STimeEvent stTimeEvent;
    stTimeEvent.ddwMillSecs = it->second;

    multiset<STimeEvent>::iterator find = m_setSTimeEvents.find(stTimeEvent);

    for(;find!=m_setSTimeEvents.end();++find)
    {

        if(find->iTimerId == iTimerId)
        {
            m_setSTimeEvents.erase(find);
            break;
        }
    }

    m_mapTimeEventIndexs.erase(it);

    return 0;

}


/** @brief (one liner)
  *
  * (documentation goes here)
  */
int CEvent::addTimer(int iTimerId,uint32_t dwExpire, const HandlerEvent &onEvent, void* pClientData)
{

	MAP_TIME_INDEX::iterator it = m_mapTimeEventIndexs.find(iTimerId);

	if(it != m_mapTimeEventIndexs.end())
	{
		delTimer(iTimerId);
	}

    STimeEvent stTimeEvent;

    stTimeEvent.ddwMillSecs = CEvent::getMillSecsNow()+dwExpire;
    stTimeEvent.iTimerId=iTimerId;
    stTimeEvent.pClientData=pClientData;
    stTimeEvent.onTimer=onEvent;

    m_mapTimeEventIndexs[iTimerId] = stTimeEvent.ddwMillSecs;

    m_setSTimeEvents.insert(stTimeEvent);

    return 0;
}


int CEvent::addMessage(int iMsgType,const HandlerEvent &onEvent,void * pClientData)
{
    SMsgEvent stMsgEvent;
    stMsgEvent.iMsgType= iMsgType;
    stMsgEvent.onMessage = onEvent;
    stMsgEvent.pClientData = pClientData;

    ::pthread_mutex_lock(&m_lock);

	m_queMsgEvents.push(stMsgEvent);

	int iFlag =write(m_iMsgFd[1],"1",1);

    ::pthread_mutex_unlock(&m_lock);

    if(iFlag < 0)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d CEvent::addMessage write error",__FILE__,__LINE__);
        return -1;
    }


    return 0;
}


int CEvent::run()
{
	if(m_bRun)
	{
		return 0;
	}
	else
	{
		m_bRun = true;
	}


    while(m_bRun)
    {

        uint64_t ddwExpire = EPOLL_WAIT_TIMEOUT;

        if (!m_setSTimeEvents.empty())
        {
            uint64_t ddwTimeNow = CEvent::getMillSecsNow();

			while(true)
			{
				multiset<STimeEvent>::iterator it=m_setSTimeEvents.begin();

				if(it == m_setSTimeEvents.end()) break;

				if (it->ddwMillSecs <= ddwTimeNow)
				{

					void *pClientData = it->pClientData;
					int iTimerId = it->iTimerId;

					HandlerEvent onTimer = it->onTimer;

					m_mapTimeEventIndexs.erase(it->iTimerId);
					m_setSTimeEvents.erase(it);

					onTimer(iTimerId,pClientData);
				}
				else
				{
					if(it != m_setSTimeEvents.end() && (it->ddwMillSecs-ddwTimeNow) > 0 )
						ddwExpire=it->ddwMillSecs-ddwTimeNow;
					break;
				}

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
                    
					while(true)
					{
						SMsgEvent stMsgEvent;
						bool bHaveMsg = false;

						::pthread_mutex_lock(&m_lock);

						int iFlag =read(m_iMsgFd[0],szBuf,1);

						if (!m_queMsgEvents.empty() && iFlag > 0)
						{
							stMsgEvent = m_queMsgEvents.front();
							m_queMsgEvents.pop();
							bHaveMsg = true;
						}
						::pthread_mutex_unlock(&m_lock);

						if(bHaveMsg)
						{
							stMsgEvent.onMessage(stMsgEvent.iMsgType,stMsgEvent.pClientData);
						}
						else
						{
							break;
						}
					}

                }
                else
                {
                    SFdEvent &stFdEvent = m_stFdEvents[m_stEPollState.stEvents[i].data.fd];
                    if((stFdEvent.iEventType&EV_READ) == EV_READ)
                        stFdEvent.onRead(m_stEPollState.stEvents[i].data.fd,stFdEvent.pClientRData);
                }
            }
            if(m_stEPollState.stEvents[i].events & EPOLLOUT)
            {

                SFdEvent &stFdEvent=m_stFdEvents[m_stEPollState.stEvents[i].data.fd];
                if((stFdEvent.iEventType&EV_WRITE) == EV_WRITE)
                    stFdEvent.onWrite(m_stEPollState.stEvents[i].data.fd,stFdEvent.pClientWData);
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
