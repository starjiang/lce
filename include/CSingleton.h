#ifndef _LCE_SINGLETON_H
#define _LCE_SINGLETON_H

#include <pthread.h>

namespace lce
{
	template<typename T>
	class CSingleton
	{
	public:
		static T& getInstance() 
		{
			if (m_pInstance == NULL) 
			{
				pthread_mutex_lock(&m_mutex);
				if (m_pInstance == NULL) 
				{
					m_pInstance = new T;
				}
				pthread_mutex_unlock(&m_mutex);
			}
			return *m_pInstance;
		}

		~CSingleton() {}

	private:
		CSingleton() {}
		CSingleton& operator=(const CSingleton&);
		CSingleton(const CSingleton&);

	private:
		static T*               m_pInstance;
		static pthread_mutex_t  m_mutex;
	};

	template<typename T>
	T* CSingleton<T>::m_pInstance = NULL;

	template<typename T>
	pthread_mutex_t CSingleton<T>::m_mutex = PTHREAD_MUTEX_INITIALIZER;

}

#endif

