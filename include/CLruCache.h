#ifndef __NCE_LRUCACHE_H__
#define __NCE_LRUCACHE_H__

#include <iostream>
#include <map>
#include <stdint.h>
#include <sys/time.h>
#include <pthread.h>
#include <tr1/unordered_map>

using namespace std;
namespace lce
{
template<typename TKey, typename TValue>
class CLruCache
{
private:

	struct SValue
	{
		uint32_t dwCTime;
		uint32_t dwExpireTime;
		uint64_t ddwAccessTime;
		TValue tValue;

	};

public:

	typedef tr1::unordered_map<TKey,SValue> MAP_CACHE;
	typedef map <uint64_t,TKey> MAP_TIME_KEY;
	typedef typename MAP_CACHE::iterator CacheIter;
	typedef typename MAP_TIME_KEY::iterator TimeIter;

	CLruCache()
	{ 
		m_dwSize = 10000; 
		::pthread_mutex_init(&m_sect, NULL);
	}

	~CLruCache()
	{
		::pthread_mutex_destroy(&m_sect);
	}

	bool init(size_t dwSize = 10000)
	{
		m_dwSize = dwSize;
		return true;
	}

	bool set(const TKey &tKey,const TValue &tValue,uint32_t dwExpireTime = 0)
	{
		::pthread_mutex_lock(&m_sect);
		if(m_mapCache.size() > m_dwSize)
		{
			int i = 0;
			for(TimeIter it = m_mapTimeKey.begin();it!=m_mapTimeKey.end();)
			{
				m_mapCache.erase(it->second);
				m_mapTimeKey.erase(it++);
				i++;
				if( i > 50) break;
			}
		}
		uint64_t ddwTime = getTickCount();
		SValue stValue;
		stValue.dwCTime = time(0);
		stValue.dwExpireTime = dwExpireTime;
		stValue.tValue = tValue;
		stValue.ddwAccessTime = ddwTime;

		CacheIter it =  m_mapCache.find(tKey);

		if(it!=m_mapCache.end())
		{
			m_mapTimeKey.erase(it->second.ddwAccessTime);
		}

		m_mapCache[tKey] = stValue;
		m_mapTimeKey[ddwTime] = tKey;

		::pthread_mutex_unlock(&m_sect);
		return true;
	}

	bool get(const TKey &tKey,TValue &tValue)
	{
		::pthread_mutex_lock(&m_sect); 
		CacheIter it = m_mapCache.find(tKey);
		if( it != m_mapCache.end())
		{
			if (it->second.dwExpireTime != 0)
			{
				uint32_t dwNow = time(0);
				if(dwNow - it->second.dwCTime > it->second.dwExpireTime)
				{
					m_mapTimeKey.erase(it->second.ddwAccessTime);
					m_mapCache.erase(tKey);
					::pthread_mutex_unlock(&m_sect);
					return false;
				}
			}

			tValue = it->second.tValue;
			uint64_t ddwOldTime =  it->second.ddwAccessTime;
			it->second.ddwAccessTime = getTickCount();

			TimeIter it2 = m_mapTimeKey.find(ddwOldTime);

			if(it2!=m_mapTimeKey.end())
			{
				m_mapTimeKey.erase(it2);
			}
			m_mapTimeKey[it->second.ddwAccessTime] = tKey;
			::pthread_mutex_unlock(&m_sect);

			return true;
		}

		::pthread_mutex_unlock(&m_sect);
		return false;
	}

	void clear()
	{
		::pthread_mutex_lock(&m_sect);
		m_mapCache.clear();
		m_mapTimeKey.clear();
		::pthread_mutex_unlock(&m_sect);
	}


	bool del(const TKey &tKey)
	{
		::pthread_mutex_lock(&m_sect);
		CacheIter it = m_mapCache.find(tKey);
		if(it!= m_mapCache.end())
		{
			m_mapTimeKey.erase(it->second.ddwAccessTime);
			m_mapCache.erase(it);
		}
		::pthread_mutex_unlock(&m_sect);
		return true;
	}

public:
	inline uint64_t getTickCount()
	{
		timeval tv;
		gettimeofday(&tv, 0);
		return tv.tv_sec * 1000000 + tv.tv_usec;
	}
private:

	::pthread_mutex_t m_sect;

	MAP_CACHE m_mapCache;
	MAP_TIME_KEY m_mapTimeKey;
	size_t m_dwSize;
};
};

#endif