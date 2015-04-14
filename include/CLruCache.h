#ifndef __NCE_LRUCACHE_H__
#define __NCE_LRUCACHE_H__

#include <iostream>
#include <map>
#include <stdint.h>
#include <sys/time.h>
#include <pthread.h>
#include "CLock.h"
#include <tr1/unordered_map>

#define CACHE_REMOVE_NUM 50

using namespace std;
namespace lce
{
template<typename TKey, typename TValue>
class CLruCache
{
private:
    struct SNode
    {
        uint32_t dwCTime;
		uint32_t dwExpireTime;
        TKey key;
        TValue data;
        SNode * next;
        SNode * prev;
    };

public:

	typedef tr1::unordered_map<TKey,SNode*> MAP_CACHE;
	typedef typename MAP_CACHE::iterator CacheIter;

	CLruCache()
	{
		m_dwMaxSize = 10000;
		m_dwSize = 0;
		pstHead = NULL;
		pstTail = NULL;
	}

	~CLruCache()
	{
	    clearList();
	}

	bool init(size_t dwMaxSize = 10000)
	{
		m_dwMaxSize = dwMaxSize;
		return true;
	}

    size_t getSize(){ return m_mapCache.size();}
    size_t getMaxSize(){ return m_dwMaxSize;}
private:
    void addToListHead(SNode * node)
    {
        if(pstHead == NULL)
        {
            node->next = NULL;
            node->prev = NULL;
            pstHead = node;
            pstTail = node;
        }
        else
        {
            node->next = pstHead;
            node->prev = NULL;
            node->next->prev = node;
            pstHead = node;
        }
        m_dwSize++;
    }

    void removeFromListTail()
    {
        if(pstTail == NULL) return;

        SNode * node = pstTail;
        pstTail = pstTail->prev;

        if(pstTail != NULL) pstTail->next = NULL;

        m_dwSize--;

        delete node;
        node = NULL;
    }

    SNode * getListBackNode()
    {
        return pstTail;
    }

    void clearList()
    {
        SNode * pstNode = pstHead;
        while(pstNode != NULL)
        {
            pstHead = pstHead->next;
            delete pstNode;
            pstNode = pstHead;
        }
        pstHead = NULL;
        pstTail = NULL;
        m_dwSize = 0;
    }

    void removeFromList(SNode *node,bool bDelete = false)
    {
        SNode *next = node->next;
        SNode *prev = node->prev;

        if(next == NULL && prev == NULL)
        {
            pstHead = NULL;
            pstTail = NULL;
        }
        else if(next != NULL &&prev == NULL)
        {
            pstHead = node->next;
            pstHead ->prev = NULL;

        }
        else if(next == NULL &&prev != NULL)
        {
            pstTail = node->prev;
            pstTail->next = NULL;
        }
        else
        {
            node->next->prev = node->prev;
            node->prev->next = node->next;
        }

        m_dwSize--;

        if(bDelete)
        {
            delete node;
            node = NULL;
        }
    }


public:
	bool set(const TKey &tKey,const TValue &tValue,uint32_t dwExpireTime = 0)
	{
		CAutoLock lock(m_oMutex);
		if(m_mapCache.size() > m_dwMaxSize)
		{
		    //如果满了，移除列表尾部CACHE_REMOVE_NUM个最久不用结点，腾出空间
		    int i = 0;
		    for(int i=0;i<CACHE_REMOVE_NUM;i++)
            {
                SNode *pstNode = getListBackNode();
                if(pstNode != NULL)
                {
                    m_mapCache.erase(pstNode->key);
                    removeFromListTail();
                }
                else
                {
                    break;
                }
            }
		}

		uint64_t ddwTime = getTickCount();
		CacheIter it =  m_mapCache.find(tKey);

		if(it!=m_mapCache.end())
		{
		    SNode *pstModifyNode = it->second;

            pstModifyNode->dwCTime = time(0);
            pstModifyNode->dwExpireTime = dwExpireTime;
            pstModifyNode->data = tValue;
            removeFromList(pstModifyNode);
            addToListHead(pstModifyNode);
		}
		else
        {
            SNode *pstNewNode = new SNode;
            pstNewNode->dwCTime = time(0);
            pstNewNode->dwExpireTime = dwExpireTime;
            pstNewNode->data = tValue;
            pstNewNode->key = tKey;
            addToListHead(pstNewNode);
            m_mapCache[tKey] = pstNewNode;

        }
		return true;
	}

	bool get(const TKey &tKey,TValue &tValue)
	{
		CAutoLock lock(m_oMutex);
		CacheIter it = m_mapCache.find(tKey);
		if( it != m_mapCache.end())
		{
			if (it->second->dwExpireTime != 0)
			{
				uint32_t dwNow = time(0);
				if(dwNow - it->second->dwCTime > it->second->dwExpireTime) //超时删除
				{
                    removeFromList(it->second,true);
					m_mapCache.erase(tKey);
					return false;
				}
			}
			//移动到列表首部
            removeFromList(it->second);
            addToListHead(it->second);
			tValue = it->second->data;

			return true;
		}
		return false;
	}

	void clear()
	{
		CAutoLock lock(m_oMutex);
		m_mapCache.clear();
		clearList();
	}


	bool del(const TKey &tKey)
	{
		CAutoLock lock(m_oMutex);
		CacheIter it = m_mapCache.find(tKey);
		if(it!= m_mapCache.end())
		{
            removeFromList(it->second,true);
			m_mapCache.erase(it);
		}
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
	CMutex m_oMutex;
	MAP_CACHE m_mapCache;
	size_t m_dwMaxSize;
	size_t m_dwSize;
    SNode * pstHead;
    SNode * pstTail;
};
};

#endif
