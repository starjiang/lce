#ifndef __LCE_CTHREAD_H
#define __LCE_CTHREAD_H

#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace lce
{
    class CThread
    {
    public:
        CThread();
        virtual ~CThread();
        int start();
        int stop();
        int detach();
        int join();

        pthread_t getId() { return m_iId; }
        pid_t getPid()
        {
            if (m_iPid == 0)
            {
                m_iPid = syscall(SYS_gettid);
            }
            return m_iPid;
        }
        bool isStoped() { return (m_iStop == 1); }
        char *getErrMsg() { return m_szErrMsg; }

        virtual int run() = 0;

    private:
        static void *procThread(void *pParam)
        {
            CThread *pCThread = (CThread *)pParam;
            pCThread->run();

            return 0;
        }

    protected:
        char m_szErrMsg[1024];

    private:
        pid_t m_iPid;
        pthread_t m_iId;
        int m_iStop;
    };
};
#endif
