#ifndef __NCE_SHM_ARRAY_H__
#define __NCE_SHM_ARRAY_H__
#include "CShm.h"
#include <stdexcept>

namespace lce
{

	template <class _Tp>
	class CShmArray
	{
		typedef _Tp value_type;
		typedef size_t size_type;
		struct SNode{
			value_type m_val;
		};
		struct SHead{
			size_type dwNodeSize;
			SNode* pNodes;
		};

		typedef SNode node_type;
	public:
		CShmArray()
			:m_pHead(NULL)
		{

		}
		~CShmArray(){

		}
		bool init(const int iShmKey, const size_t dwNodeSize, const bool bCreate=true, const bool bReadOnly=false);
		bool isCreate() const {	return m_oShm.isCreate();	}

		inline value_type& operator[](const size_t dwIndex);
		inline const value_type& operator[](const size_t dwIndex) const;

		size_type size() const	{	return m_pHead->dwNodeSize;	}

		const char* getErrMsg() const {	return m_szErrMsg;	}
	private:
		inline char* _getAddr(const void* pOffset){
			if ( pOffset == 0 )
				return 0;
			return (char*)m_pHead + (size_type)pOffset;
		}
		inline char* _getOffset(const void* ptr) const {
			if ( ptr == NULL )
				return 0;
			return (char*)ptr - (size_type)m_pHead;
		}

	private:
		SHead* m_pHead;
		node_type* m_pNodes;
		lce::CShm m_oShm;
		char m_szErrMsg[1024];
	};

	template<class _Tp>
	typename CShmArray<_Tp>::value_type& CShmArray<_Tp>::operator[](const size_t dwIndex)
	{
		node_type* pNode = m_pNodes+dwIndex;
		return pNode->m_val;
	}

	template<class _Tp>
	const typename CShmArray<_Tp>::value_type& CShmArray<_Tp>::operator[](const size_t dwIndex) const
	{
		node_type* pNode = m_pNodes+dwIndex;
		return pNode->m_val;
	}


	template<class _Tp>
	bool CShmArray<_Tp>::init(const int iShmKey, const size_t dwNodeSize, const bool bCreate/*=true*/, const bool bReadOnly /*=false*/)
	{
		size_type dwShmMaxSize = dwNodeSize*sizeof(value_type)+sizeof(SHead);
		//create shm
		if ( m_oShm.getShmID() <= 0 || !m_oShm.attach() )
		{
			if ( !m_oShm.create(iShmKey, dwShmMaxSize, bCreate, bReadOnly) )
			{
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm init create error: %s", m_oShm.getErrMsg());
				return false;
			}
		}

		if ( m_oShm.isCreate() )
		{
			char* pShmBuf = reinterpret_cast<char*>(m_oShm.getShmBuf());

			//shm head
			m_pHead = (SHead*)pShmBuf;

			m_pHead->dwNodeSize = dwNodeSize;

			if ( dwShmMaxSize <= sizeof(SHead) )
			{
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm max size<%lu> is small.", dwShmMaxSize);
				return false;
			}
			m_pHead->pNodes = (node_type*)(sizeof(SHead));
			m_pNodes = (node_type*)this->_getAddr(m_pHead->pNodes);
		}
		else
		{
			m_pHead = (SHead*)m_oShm.getShmBuf();
			m_pNodes = (node_type*)this->_getAddr(m_pHead->pNodes);
		}

		return true;
	}

};


#endif

