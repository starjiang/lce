#ifndef __NCE_CEVENT_H
#define __NCE_CEVENT_H

#include <sys/epoll.h>
#include <stddef.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <queue>
#include <set>
#include <algorithm>
#include <fcntl.h>
using namespace std;

namespace lce
{
typedef void (*fdEventCb)(int iFd,void *pData);
typedef void (*timeEventCb)(uint32_t dwTimeId,void *pData);
typedef void (*msgEventCb)(uint32_t dwMsgType,void *pData);
const int EPOLL_MAX_SIZE = 500000;
const int EPOLL_MAX_EVENT = 1024;
const int EPOLL_WAIT_TIMEOUT = 1000;
const int CEVENT_MAX_TIME_EVENT =10000;

class CEvent
{
public:
    enum EventType
    {
        EV_DONE=0,
        EV_READ=1,
        EV_WRITE=2,

    };
    typedef struct
    {
        int iEPollFd;
        struct epoll_event stEvents[EPOLL_MAX_EVENT];
    } SEPollState;

    typedef struct
    {
        int iEventType; /* one of EV_(READ|WRITE) */
        fdEventCb pReadProc;
        fdEventCb pWriteProc;
        void *pClientData;
    } SFdEvent;

    typedef struct
    {
        uint32_t dwTimerId; /* one of EV_(READ|WRITE) */
        uint64_t ddwMillSecs;
        timeEventCb pTimeProc;
        void *pClientData;
    } STimeEvent;

    typedef struct
    {
        uint32_t dwMsgType; /* one of EV_(READ|WRITE) */
        msgEventCb pMsgProc;
        void *pClientData;
    } SMsgEvent;


    struct cmpTimer
    {
        bool operator () (const STimeEvent *stTimeEvent1,const STimeEvent *stTimeEvent2)
        {
            return stTimeEvent1->ddwMillSecs < stTimeEvent2->ddwMillSecs;
        }

    };


public:
    CEvent();
    ~CEvent();

    int init();
    int addFdEvent(int iWatchFd,int iEventType,fdEventCb pFdCb,void * pClientData);
    int delFdEvent(int iWatchFd,int iEventType);

    int addTimer(uint32_t dwTimerId,uint64_t ddwExpire,timeEventCb pTimeCb,void * pClientData);
    int delTimer(uint32_t dwTimerId);
    int addMessage(uint32_t dwMsgType,msgEventCb pMsgCb,void * pClientData);

    int run();
    int stop();
    const char * getErrorMsg(){ return m_szErrMsg; }
private:


    static inline uint64_t  getMillSecsNow()
    {
        struct timeval stNow;
        gettimeofday(&stNow, NULL);
        return (stNow.tv_sec*1000+stNow.tv_usec/1000);
    }

private:

    int m_iMsgFd[2];
    SFdEvent *m_stFdEvents;
    SEPollState m_stEPollState;
    queue <SMsgEvent *> m_queMsgEvents;
    multiset <STimeEvent*,cmpTimer> m_setSTimeEvents;
    uint64_t m_szTimeEventIndexs[CEVENT_MAX_TIME_EVENT];
    uint32_t m_dwTimerNum;
    char m_szErrMsg[1024];
    bool m_bRun;

    pthread_mutex_t  m_lock;

};
};
#endif
