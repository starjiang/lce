#ifndef __NCE_CTHREAD_H
#define __NCE_CTHREAD_H

#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
namespace lce
{
class CThread
{
public:
    CThread();
    ~CThread();
    int start();
    int stop();

    pthread_t getId(){ return m_iId; }

    bool isStoped(){ return (m_iStop == 1);}
    char * getErrMsg(){ return m_szErrMsg; }

    virtual int run() = 0 ;
private:
    static void * procThread(void * pParam)
    {
        CThread * pCThread =(CThread*)pParam;
        pCThread->run();
        return 0;
    }
protected:
    char m_szErrMsg[1024];
private:

    pthread_t m_iId;
    int m_iStop;

};
};
#endif
