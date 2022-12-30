#include "CThread.h"

namespace lce
{

    CThread::CThread()
    {
        m_iId = 0;
        m_iPid = 0;
        m_iStop = 0;
    }

    CThread::~CThread()
    {
        detach();
        stop();
    }

    int CThread::start()
    {

        // ���ζ��߳���SIGPIPE�ź�,��ֹ�����˳�
        sigset_t signal_mask;
        sigemptyset(&signal_mask);
        sigaddset(&signal_mask, SIGPIPE);
        pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);

        if (pthread_create(&m_iId, 0, CThread::procThread, this) != 0)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "file:%s,line:%d,errno=%d,error=%s", __FILE__, __LINE__, errno, strerror(errno));
            return -1;
        }
        m_iStop = 0;

        return 0;
    }

    int CThread::detach()
    {
        if (m_iId > 0)
        {
            return pthread_detach(m_iId);
        }

        return 0;
    }

    int CThread::join()
    {
        if (m_iId > 0)
        {
            return pthread_join(m_iId, NULL);
        }

        return 0;
    }

    int CThread::stop()
    {
        if (m_iStop)
            return 0;

        m_iStop = 1;
        if (m_iId > 0)
        {
            return pthread_cancel(m_iId);
        }

        return 0;
    }

};
