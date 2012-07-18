#ifndef __LDF_HASH_MAP_H__
#define __LDF_HASH_MAP_H__

#include <utility>
#include <memory>
#include <cassert>
using std::pair;

namespace lce
{

	struct SHashNodeBase{
		SHashNodeBase* pHashNext;	//hash列表使用
		SHashNodeBase* pHashPre;	//hash列表使用
		SHashNodeBase* pListNext;	//空闲或使用列表指针使用
		SHashNodeBase* pListPre;	//空闲或使用列表指针使用
	};

	template<typename T>
	struct SHashNode: public SHashNodeBase{
		SHashNode(){}
		unsigned long dwKey;	//key
		T  tVal;				//数据
		size_t dwText;	//test 数据
	};

	//iterator
	template<typename T>
	class CHashMapIterator
	{
		//	friend class CHashMap<T>;
	public:
		typedef CHashMapIterator<T> _Self;
	public:
		inline CHashMapIterator();
		inline ~CHashMapIterator();
		inline CHashMapIterator(SHashNodeBase* pValue);
		inline _Self& operator=(const _Self& rhs);
		inline _Self& operator=(SHashNodeBase* pValue);
		inline CHashMapIterator(const _Self& rhs);
		inline _Self& operator++();
		inline _Self operator++(int);
		inline _Self& operator--();
		inline _Self operator--(int);
		inline T& operator*();
		inline T* operator&();
		inline T* operator->();
		bool operator==(const _Self& it);
		bool operator!=(const _Self& it);
		SHashNodeBase* m_pValue;
	private:

	};


	template<typename T>
	class CHashMap
	{
	public:
		typedef T value_type;
		typedef value_type* pointer;
		typedef value_type& reference;
		typedef size_t size_type;
		typedef ptrdiff_t defference_type;
		typedef SHashNode<T> node_type;
		typedef CHashMapIterator<T> Iterator;
	public:
		inline CHashMap(void);
		inline ~CHashMap(void);

		inline bool init(const size_type dwMaxSize = 10000);

		inline bool empty() const;
		inline bool full() const;
		inline Iterator begin()
		{
			return reinterpret_cast<SHashNode<T>*>(m_pListHead->pListNext);
		}
		inline Iterator end()	{	return reinterpret_cast<SHashNode<T>*>(m_pListHead);	}
		inline pair<Iterator, bool> insert(const unsigned long dwKey,const T& tVal);
		inline Iterator find(const unsigned long dwKey);

		inline void erase(const unsigned long dwKey);
		inline void erase(Iterator it);
		inline void clear();

		//test
		inline void testEmptySize();
		inline void testSize();

		size_type size() const {	return m_dwSize;	}
		size_type maxSize() const {	return m_dwMaxSize;	}
		const char* getErrMsg() const {	return m_szErrMsg;	}
	private:
		inline void free(node_type* pNode);
		inline node_type* malloc();
		inline size_type getHashPos(const unsigned long dwKey)
		{
			return dwKey%m_dwHashKey;
		}
		inline void insertList(node_type* pNode);
		inline void removeList(node_type* pNode);
	private:
		char m_szErrMsg[1024];
		SHashNodeBase* m_pHashTable;
		node_type* m_pData;
		node_type* m_pEmpty;
		SHashNodeBase* m_pListHead;
		size_type m_dwSize;		//当前使用数
		size_type m_dwMaxSize;	//最大可容纳数
		size_type m_dwHashKey;
	};



	template<typename T>
	CHashMap<T>::CHashMap(void)
	{
		m_dwMaxSize = 0;
		m_dwSize = 0;
		memset(m_szErrMsg, 0, sizeof(m_szErrMsg));
		m_pData = NULL;
		m_pEmpty = NULL;
		m_pHashTable = NULL;
		m_dwHashKey = 10000;
		m_pListHead = NULL;
	}

	template<typename T>
	CHashMap<T>::~CHashMap(void)
	{
		if (NULL != m_pData)
		{
			delete[] m_pData;
		}

		if (NULL != m_pHashTable)
		{
			delete[] m_pHashTable;
		}

		if (NULL != m_pListHead)
		{
			delete m_pListHead;
		}
	}

	template<typename T>
	bool CHashMap<T>::init(const size_type dwMaxSize /* = 10000*/)
	{
		m_pData = new node_type[dwMaxSize];
		if (NULL == m_pData)
		{
			return false;
		}

		memset(m_pData, 0, sizeof(node_type)*dwMaxSize);

		//把内存连到空闲列表里
		for (size_type i=0; i<dwMaxSize; ++i)
		{
			(m_pData+i)->dwText = i;
			if (NULL == m_pEmpty)
			{
				(m_pData+i)->pHashNext = NULL;
				(m_pData+i)->pHashPre = NULL;
				(m_pData+i)->pListNext = NULL;
				(m_pData+i)->pListPre = NULL;
				m_pEmpty = m_pData+i;
			}
			else
			{
				(m_pData+i)->pListNext = reinterpret_cast<SHashNodeBase*>(m_pEmpty);
				m_pEmpty->pListPre = reinterpret_cast<SHashNodeBase*>(m_pData+i);
				m_pEmpty = m_pData+i;
				(m_pData+i)->pListPre = NULL;
				(m_pData+i)->pHashNext = NULL;
				(m_pData+i)->pHashPre = NULL;
			}
		}

		m_dwMaxSize = dwMaxSize;
		m_dwHashKey = dwMaxSize;
		m_pHashTable = new SHashNodeBase[m_dwHashKey];
		if (NULL == m_pHashTable)
		{
			return false;
		}
		for (size_type i=0; i<m_dwHashKey; ++i)
		{
			(m_pHashTable+i)->pHashNext = m_pHashTable+i;
			(m_pHashTable+i)->pHashPre = m_pHashTable+i;
		}

		m_pListHead = new SHashNodeBase;
		if (NULL == m_pListHead)
		{
			return false;
		}
		m_pListHead->pListNext = m_pListHead;
		m_pListHead->pListPre = m_pListHead;

		return true;
	}

	template<typename T>
	void CHashMap<T>::insertList(node_type* pNode)
	{
		pNode->pListNext = m_pListHead->pListNext;
		m_pListHead->pListNext->pListPre = pNode;
		pNode->pListPre = m_pListHead;
		m_pListHead->pListNext = pNode;
	}

	template<typename T>
	void CHashMap<T>::removeList(node_type* pNode)
	{
		assert(pNode->pListNext != pNode);
		assert(pNode->pListPre != pNode);

		pNode->pListNext->pListPre = pNode->pListPre;
		pNode->pListPre->pListNext = pNode->pListNext;
	}

	template<typename T>
	SHashNode<T>* CHashMap<T>::malloc()
	{
		node_type* pNode = NULL;
		if (NULL != m_pEmpty)
		{
			pNode = m_pEmpty;
			m_pEmpty = reinterpret_cast<node_type*>(m_pEmpty->pListNext);
			if (NULL != m_pEmpty)
			{
				m_pEmpty->pListPre = NULL;
			}
			pNode->pListNext = NULL;
			pNode->pListPre = NULL;
			++m_dwSize;

			//加入使用列表
			this->insertList(pNode);
		}
		else
		{
			//		xsnprintf(m_szErrMsg,sizeof(m_szErrMsg),"no space.");
		}
		return pNode;
	}

	template<typename T>
	void CHashMap<T>::free(node_type* pNode)
	{
		this->RemoveList(pNode);
		memset(pNode,0,sizeof(node_type));
		if (NULL == m_pEmpty)
		{
			m_pEmpty = pNode;
			pNode->pListNext = NULL;
			pNode->pListPre = NULL;
		}
		else
		{
			pNode->pListNext = m_pEmpty;
			m_pEmpty->pListPre = pNode;
			m_pEmpty = pNode;
		}
		--m_dwSize;

	}

	template<typename T>
	pair<CHashMapIterator<T>, bool> CHashMap<T>::insert(const unsigned long dwKey,const T& tVal)
	{
		pair<CHashMapIterator<T>, bool> rePair(end(),false);
		size_type dwHashPos = this->getHashPos(dwKey);


		bool bHas = false;
		SHashNodeBase* pHeadNode = m_pHashTable+dwHashPos;
		SHashNodeBase* pTmpNode = pHeadNode->pHashNext;
		while (pHeadNode != pTmpNode)
		{
			if(reinterpret_cast<node_type*>(pTmpNode)->dwKey == dwKey)
			{
				reinterpret_cast<node_type*>(pTmpNode)->tVal = tVal;
				rePair.second = true;
				rePair.first = pTmpNode;
				bHas = true;
				break;
			}
			pTmpNode = pTmpNode->pHashNext;
		}

		if (!bHas)
		{
			SHashNodeBase* pNewNode = reinterpret_cast<SHashNodeBase*>(this->malloc());
			if (NULL != pNewNode)
			{
				reinterpret_cast<node_type*>(pNewNode)->dwKey = dwKey;
				reinterpret_cast<node_type*>(pNewNode)->tVal = tVal;
				rePair.second = true;
				rePair.first = pNewNode;

				pNewNode->pHashNext = pHeadNode->pHashNext;
				pHeadNode->pHashNext->pHashPre = pNewNode;
				pNewNode->pHashPre = pHeadNode;
				pHeadNode->pHashNext = pNewNode;
			}
			else
			{
				rePair.second = false;
			}
		}

		return rePair;
	}

	template<typename T>
	CHashMapIterator<T> CHashMap<T>::find(const unsigned long dwKey)
	{
		SHashNodeBase* pVal = m_pListHead;
		size_type dwHashPos = this->GetHashPos(dwKey);
		SHashNodeBase* pHeadNode = m_pHashTable+dwHashPos;
		SHashNodeBase* pTmpNode = pHeadNode->pHashNext;
		while (pHeadNode != pTmpNode)
		{
			if(reinterpret_cast<node_type*>(pTmpNode)->dwKey == dwKey)
			{
				pVal = pTmpNode;
				break;
			}
			pTmpNode = pTmpNode->pHashNext;
		}
		return  pVal;
	}

	template<typename T>
	void CHashMap<T>::erase(const unsigned long dwKey)
	{
		size_type dwHashPos = this->getHashPos(dwKey);
		SHashNodeBase* pHeadNode = m_pHashTable+dwHashPos;
		SHashNodeBase* pTmpNode = pHeadNode->pHashNext;
		while (pHeadNode != pTmpNode)
		{
			if(reinterpret_cast<node_type*>(pTmpNode)->dwKey == dwKey)
			{
				pTmpNode->pHashNext->pHashPre = pTmpNode->pHashPre;
				pTmpNode->pHashPre->pHashNext = pTmpNode->pHashNext;
				this->Free(reinterpret_cast<node_type*>(pTmpNode));
				break;
			}
			pTmpNode = pTmpNode->pHashNext;
		}
	}

	template<typename T>
	void CHashMap<T>::erase(Iterator it)
	{
		if (it != this->End())
		{
			SHashNodeBase* pTmpNode = it.m_pValue;
			pTmpNode->pHashNext->pHashPre = pTmpNode->pHashPre;
			pTmpNode->pHashPre->pHashNext = pTmpNode->pHashNext;
			this->Free(reinterpret_cast<node_type*>(pTmpNode));
		}
	}

	template<typename T>
	void CHashMap<T>::clear()
	{
		for (size_type i=0; i<m_dwHashKey; ++i)
		{
			(m_pHashTable+i)->pHashNext = m_pHashTable+i;
			(m_pHashTable+i)->pHashPre = m_pHashTable+i;
		}

		m_dwSize = 0;

		if (m_pListHead->pListNext != m_pListHead)
		{
			if (NULL != m_pEmpty)
			{
				m_pListHead->pListNext->pListPre = NULL;
				m_pListHead->pListPre->pListNext = m_pEmpty;
				m_pEmpty = reinterpret_cast<node_type*>(m_pListHead->pListNext);
			}
			else
			{
				m_pEmpty = reinterpret_cast<node_type*>(m_pListHead->pListNext);
				m_pListHead->pListNext->pListPre = NULL;
				m_pListHead->pListPre->pListNext = NULL;
			}

			m_pListHead->pListNext = m_pListHead;
			m_pListHead->pListPre = m_pListHead;
		}
	}

	template<typename T>
	bool CHashMap<T>::empty() const
	{
#ifndef NDEBUG
		if (m_pListHead->pListNext == m_pListHead)
		{
			assert(m_dwSize == 0);
		}
		else
		{
			assert(m_dwSize != 0);
		}
#endif

		return m_pListHead->pListNext == m_pListHead ? true : false;
	}

	template<typename T>
	bool CHashMap<T>::full() const
	{
#ifndef NDEBUG
		if (m_pEmpty == NULL)
		{
			assert(m_dwSize == m_dwMaxSize);
		}
		else
		{
			assert(m_dwSize != m_dwMaxSize);
		}
#endif
		return m_pEmpty == NULL ? true : false;
	}

	//test
	template<typename T>
	void CHashMap<T>::testEmptySize()
	{
#ifndef NDEBUG
		size_type tmpSize = 0;

		SHashNodeBase* pTmpEmptyHead = reinterpret_cast<SHashNodeBase*>(m_pEmpty);

		while(pTmpEmptyHead)
		{
			tmpSize++;
			pTmpEmptyHead = pTmpEmptyHead->pListNext;
		}

		assert(tmpSize == m_dwMaxSize-m_dwSize);
#endif
	}

	template<typename T>
	void CHashMap<T>::testSize()
	{
#ifndef NDEBUG
		size_type tmpSize = 0;

		SHashNodeBase* pTmpListHead = m_pListHead;

		while(pTmpListHead->pListNext != m_pListHead)
		{
			tmpSize++;
			pTmpListHead = pTmpListHead->pListNext;
		}

		assert(tmpSize == m_dwSize);
#endif
	}


	//////////////////////////////////////////////////////////////////////////
	//HashMap iterator

	template<typename T>
	CHashMapIterator<T>::CHashMapIterator()
		:m_pValue(NULL)
	{
	}

	template<typename T>
	CHashMapIterator<T>::~CHashMapIterator()
	{
	}

	template<typename T>
	CHashMapIterator<T>::CHashMapIterator(SHashNodeBase* pValue)
		:m_pValue(pValue)
	{
	}

	template<typename T>
	CHashMapIterator<T>& CHashMapIterator<T>::operator=(const CHashMapIterator<T>& rhs)
	{
		if (this != &rhs)
		{
			m_pValue = rhs.m_pValue;
		}
		return *this;
	}

	template<typename T>
	CHashMapIterator<T>& CHashMapIterator<T>::operator=(SHashNodeBase* pValue)
	{
		m_pValue = pValue;
		return *this;
	}

	template<typename T>
	CHashMapIterator<T>::CHashMapIterator(const CHashMapIterator<T>& rhs)
	{
		m_pValue = rhs.m_pValue;
	}

	template<typename T>
	CHashMapIterator<T>& CHashMapIterator<T>::operator++()
	{
		m_pValue = m_pValue->pListNext;
		return *this;
	}

	template<typename T>
	CHashMapIterator<T> CHashMapIterator<T>::operator++(int)
	{
		CHashMapIterator<T> tmp = *this;
		m_pValue = m_pValue->pListNext;
		return tmp;
	}

	template<typename T>
	CHashMapIterator<T>& CHashMapIterator<T>::operator--()
	{
		m_pValue = m_pValue->pListPre;
		return *this;
	}

	template<typename T>
	CHashMapIterator<T> CHashMapIterator<T>::operator--(int)
	{
		CHashMapIterator<T> tmp = *this;
		m_pValue = m_pValue->pListPre;
		return tmp;
	}

	template<typename T>
	T& CHashMapIterator<T>::operator*()
	{
		return reinterpret_cast<SHashNode<T>*>(m_pValue)->tVal;
	}

	template<typename T>
	T* CHashMapIterator<T>::operator->()
	{
		return &(reinterpret_cast<SHashNode<T>*>(m_pValue)->tVal);
	}

	template<typename T>
	T* CHashMapIterator<T>::operator&()
	{
		return &(reinterpret_cast<SHashNode<T>*>(m_pValue)->tVal);
	}

	template<typename T>
	bool CHashMapIterator<T>::operator==(const CHashMapIterator<T>& it)
	{
		return m_pValue == it.m_pValue;
	}

	template<typename T>
	bool CHashMapIterator<T>::operator!=(const CHashMapIterator<T>& it)
	{
		return m_pValue != it.m_pValue;
	}


};

#endif
