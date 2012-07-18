#ifndef __NCE_CLOCK_H
#define __NCE_CLOCK_H

#include <pthread.h>

namespace lce
{

class CLock
{
public:

	CLock(void)
	{
	}

	virtual bool lock() = 0;
	virtual bool unlock() = 0;

	virtual ~CLock(void)
	{
	}
};

class CMutex : public CLock
{
public:
	inline CMutex(void)
	{
		::pthread_mutex_init(&m_lock,0);
	}
	inline ~CMutex(void)
	{
		::pthread_mutex_destroy(&m_lock);
	}

	inline virtual bool lock()
	{

		::pthread_mutex_lock(&m_lock);
		return true;
	};

	inline virtual bool unlock()
	{
        ::pthread_mutex_unlock(&m_lock);
		return true;
	}
private:
	CMutex(const CMutex& rhs);
	CMutex& operator=(const CMutex& rhs);
private:
	char m_szErrMsg[1024];
	pthread_mutex_t  m_lock;

};

class CAutoLock
{
public:

	CAutoLock(CLock& lock)
		:m_lock(lock)
	{
		m_lock.lock();
	}

	virtual ~CAutoLock(void)
	{
		m_lock.unlock();
	}
private:
	CLock& m_lock;
};

};

#endif



