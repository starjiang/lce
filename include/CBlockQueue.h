#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
using namespace std;

namespace lce
{

template<class T>
class CBlockQueue
{
public:
    CBlockQueue(int dwMaxSize = 1000)
    {
        if(dwMaxSize <= 0)
        {
            dwMaxSize = 1000;
        }

        m_dwMaxSize = dwMaxSize;
        m_pData = new T[dwMaxSize];
        m_dwSize = 0;
        m_dwFront = -1;
        m_dwBack = -1;

        m_mutex = new pthread_mutex_t;
        m_cond = new pthread_cond_t;
        pthread_mutex_init(m_mutex, 0);
        pthread_cond_init(m_cond, 0);
    }

    void clear()
    {
        pthread_mutex_lock(m_mutex);
        m_dwSize = 0;
        m_dwFront = -1;
        m_dwBack = -1;
        pthread_mutex_unlock(m_mutex);
    }

    ~CBlockQueue()
    {
        pthread_mutex_lock(m_mutex);

        if(m_pData != NULL)	delete  []m_pData;

        pthread_mutex_unlock(m_mutex);

        pthread_mutex_destroy(m_mutex);
        pthread_cond_destroy(m_cond);

        delete m_mutex;
        delete m_cond;
    }

    bool full() const
    {
        pthread_mutex_lock(m_mutex);
        if(m_dwSize >= m_dwMaxSize)
        {
            pthread_mutex_unlock(m_mutex);
            return true;
        }
        pthread_mutex_unlock(m_mutex);
        return false;
    }

    bool empty() const
    {
        pthread_mutex_lock(m_mutex);
        if(0 == m_dwSize)
        {
            pthread_mutex_unlock(m_mutex);
            return true;
        }
        pthread_mutex_unlock(m_mutex);
        return false;
    }

    bool front(T& value)const
    {
        pthread_mutex_lock(m_mutex);
        if(0 == m_dwSize)
        {
            pthread_mutex_unlock(m_mutex);
            return false;
        }
        value = m_pData[m_dwFront];
        pthread_mutex_unlock(m_mutex);
        return true;
    }

    bool back(T& value)const
    {
        pthread_mutex_lock(m_mutex);
        if(0 == m_dwSize)
        {
            pthread_mutex_unlock(m_mutex);
            return false;
        }
        value = m_pData[m_dwBack];
        pthread_mutex_unlock(m_mutex);
        return true;
    }

    int size()const
    {
        int dwSize = 0;
        pthread_mutex_lock(m_mutex);
        dwSize = m_dwSize;
        pthread_mutex_unlock(m_mutex);
        return dwSize;
    }

    int max_size()const
    {
        return m_dwMaxSize;

    }

    bool push(const T& item)
    {
        while(true)
        {
            pthread_mutex_lock(m_mutex);
            if(m_dwSize >= m_dwMaxSize)
            {
                pthread_cond_wait(m_cond, m_mutex);
                pthread_mutex_unlock(m_mutex);
                continue;
            }
            else
            {
                break;
            }
        }

        m_dwBack = (m_dwBack + 1) % m_dwMaxSize;
        m_pData[m_dwBack] = item;

        m_dwSize++;
        pthread_mutex_unlock(m_mutex);
        pthread_cond_broadcast(m_cond);

        return true;
    }

    bool pop(T& item)
    {
        pthread_mutex_lock(m_mutex);
        while(true)
        {
            if(m_dwSize <= 0)
            {
                pthread_cond_wait(m_cond, m_mutex);
                pthread_mutex_unlock(m_mutex);
                continue;
            }
            else
            {
                break;
            }

        }

        m_dwFront = (m_dwFront + 1) % m_dwMaxSize;
        item = m_pData[m_dwFront];
        m_dwSize--;
        pthread_mutex_unlock(m_mutex);
        pthread_cond_broadcast(m_cond);
        return true;
    }

    bool pop(T& item, int timeout)
    {
        struct timespec t = {0,0};
        struct timeval now = {0,0};
        gettimeofday(&now, 0);
        pthread_mutex_lock(m_mutex);
        if(m_dwSize <= 0)
        {
            t.tv_sec = now.tv_sec + timeout/1000;
            t.tv_nsec = (timeout % 1000)*1000;
            if(0 != pthread_cond_timedwait(m_cond, m_mutex, &t))
            {
                pthread_mutex_unlock(m_mutex);
                return false;
            }
        }

        if(m_dwSize <= 0)
        {
            pthread_mutex_unlock(m_mutex);
            return false;
        }

        m_dwFront = (m_dwFront + 1) % m_dwMaxSize;
        item = m_pData[m_dwFront];
        m_dwSize--;
        pthread_mutex_unlock(m_mutex);
        pthread_cond_broadcast(m_cond);
        return true;
    }

private:
    pthread_mutex_t *m_mutex;
    pthread_cond_t *m_cond;
    T *m_pData;
    int m_dwSize;
    int m_dwMaxSize;
    int m_dwFront;
    int m_dwBack;
};
};
#endif
