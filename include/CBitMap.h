#ifndef __NCE_SHM_BITMAP_H__
#define __NCE_SHM_BITMAP_H__
#include "CShm.h"


namespace lce
{

	template<size_t _bit>
	class CBitmap{};

	//0-1
	template<> class CBitmap<1>
	{
		typedef size_t size_type;
		struct SHead{
			size_type dwShmSize;
		};

	public:
		CBitmap(){}
		~CBitmap(){}

		bool init(const int iShmKey, const unsigned long dwBitmapSize, const bool bCreate=true, const bool bReadOnly=false){
			size_type dwShmSize = (dwBitmapSize+7)/8 + sizeof(SHead);
			m_dwMaxIndex = dwBitmapSize;

			//create shm
			if ( m_oShm.getShmID() <= 0 || !m_oShm.attach() )
			{
				if ( !m_oShm.create(iShmKey, dwShmSize, bCreate, bReadOnly) )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm init create error: %s", m_oShm.getErrMsg());
					return false;
				}
			}

			char* pShmBuf = reinterpret_cast<char*>(m_oShm.getShmBuf());
			if ( m_oShm.isCreate() )
			{
				//shm head
				m_pHead = (SHead*)pShmBuf;

				m_pHead->dwShmSize = dwShmSize;

				if ( dwShmSize <= sizeof(SHead) )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm max size<%lu> is small.", dwShmSize);
					return false;
				}
				m_pDataPtr = (char*)((char*)pShmBuf+sizeof(SHead));
			}
			else
			{
				m_pHead = (SHead*)m_oShm.getShmBuf();
				m_pDataPtr = (char*)((char*)pShmBuf+sizeof(SHead));
			}

			return true;
		}

		bool get(const unsigned long dwIndex) const {
			if ( dwIndex > m_dwMaxIndex )
			{
				return false;
			}

			char cFlag;
			cFlag = *(m_pDataPtr+dwIndex/ 8) >> (dwIndex % 8);
			cFlag = cFlag & 0x1;
			return (cFlag ==1 ? true:false);
		}

		bool set(const unsigned long dwIndex, const bool bVal){
			int  iIndexBitPos;
			static unsigned char caUinFlag[] = {0xfe, 0xfd, 0xfb, 0xf7,0xef,0xdf,0xbf,0x7f};
			char cFlag = bVal ? 1 : 0;

			if ( dwIndex > m_dwMaxIndex )
			{
				return false;
			}

			iIndexBitPos = dwIndex % 8;


			cFlag = cFlag << (iIndexBitPos);

			*(m_pDataPtr+dwIndex/8) &= caUinFlag[iIndexBitPos];		//清位为0
			*(m_pDataPtr+dwIndex/8) |= cFlag;//

			return true;
		}

		void clear()	{	memset(m_pDataPtr, 0, shm_size()-sizeof(SHead));	}

		const char* data() const {	return (char*)m_pDataPtr;	}
		size_t size() const {	return m_dwMaxIndex;	}
		size_t shm_size() const {	return m_pHead->dwShmSize;	}
	protected:
	private:
		SHead* m_pHead;
		char* m_pDataPtr;
		unsigned long m_dwMaxIndex;
		CShm m_oShm;
		char m_szErrMsg[1024];

	};


	//0-3
	template<> class CBitmap<2>
	{
		typedef size_t size_type;
		struct SHead{
			size_type dwShmSize;
		};

	public:
		CBitmap(){}
		~CBitmap(){}

		bool init(const int iShmKey, const unsigned long dwBitmapSize, const bool bCreate=true, const bool bReadOnly=false){
			size_type dwShmSize = ((dwBitmapSize+7)/8)*2 + sizeof(SHead);
			m_dwMaxIndex = dwBitmapSize;

			//create shm
			if ( m_oShm.getShmID() <= 0 || !m_oShm.attach() )
			{
				if ( !m_oShm.create(iShmKey,dwShmSize, bCreate, bReadOnly) )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm init create error: %s", m_oShm.getErrMsg());
					return false;
				}
			}

			char* pShmBuf = reinterpret_cast<char*>(m_oShm.getShmBuf());
			if ( m_oShm.isCreate() )
			{
				//shm head
				m_pHead = (SHead*)pShmBuf;

				m_pHead->dwShmSize = dwShmSize;

				if ( dwShmSize <= sizeof(SHead) )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm max size<%lu> is small.", dwShmSize);
					return false;
				}
				m_pDataPtr = (char*)((char*)pShmBuf+sizeof(SHead));
			}
			else
			{
				m_pHead = (SHead*)m_oShm.getShmBuf();
				m_pDataPtr = (char*)((char*)pShmBuf+sizeof(SHead));
			}

			return true;
		}

		int get(const unsigned long dwIndex)  const {
			if ( dwIndex > m_dwMaxIndex )
			{
				return false;
			}

			unsigned char ucFlag;
			ucFlag = *(m_pDataPtr+dwIndex/ 4) >> (dwIndex%4*2);
			ucFlag = ucFlag & 0x3;
			return ucFlag;
		}

		bool set(const unsigned long dwIndex, const unsigned char ucVal){
			int  iIndexBitPos;
			static unsigned char caUinFlag[] = {0xfc, 0xf3, 0xcf, 0x3f};
			unsigned long ucFlag = ucVal;

			if ( dwIndex > m_dwMaxIndex )
			{
				return false;
			}

			iIndexBitPos = dwIndex%4;


			ucFlag = ucFlag << (iIndexBitPos*2);

			*(m_pDataPtr+dwIndex/4) &= caUinFlag[iIndexBitPos];		//清位为0
			*(m_pDataPtr+dwIndex/4) |= ucFlag;//

			return true;
		}
		void clear()	{	memset(m_pDataPtr, 0, shm_size()-sizeof(SHead));	}
		const char* data() const {	return (char*)m_pDataPtr;	}
		size_t size() const {	return m_dwMaxIndex;	}
		size_t shm_size() const {	return m_pHead->dwShmSize;	}
	protected:
	private:
		SHead* m_pHead;
		char* m_pDataPtr;
		unsigned long m_dwMaxIndex;
		CShm m_oShm;
		char m_szErrMsg[1024];

	};


	//0-7
	template<> class CBitmap<3>
	{
		typedef size_t size_type;
		struct SHead{
			size_type dwShmSize;
		};

	public:
		CBitmap(){}
		~CBitmap(){}

		bool init(const int iShmKey, const unsigned long dwBitmapSize, const bool bCreate=true, const bool bReadOnly=false){
			size_type dwShmSize = ((dwBitmapSize+7)/8)*3 + sizeof(SHead);
			m_dwMaxIndex = dwBitmapSize;

			//create shm
			if ( m_oShm.getShmID() <= 0 || !m_oShm.attach() )
			{
				if ( !m_oShm.create(iShmKey, dwShmSize, bCreate, bReadOnly) )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm init create error: %s", m_oShm.getErrMsg());
					return false;
				}
			}

			char* pShmBuf = reinterpret_cast<char*>(m_oShm.getShmBuf());
			if ( m_oShm.isCreate() )
			{
				//shm head
				m_pHead = (SHead*)pShmBuf;

				m_pHead->dwShmSize = dwShmSize;

				if ( dwShmSize <= sizeof(SHead) )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm max size<%lu> is small.", dwShmSize);
					return false;
				}
				m_pDataPtr = (char*)((char*)pShmBuf+sizeof(SHead));
			}
			else
			{
				m_pHead = (SHead*)m_oShm.getShmBuf();
				m_pDataPtr = (char*)((char*)pShmBuf+sizeof(SHead));
			}

			return true;
		}

		int get(const unsigned long dwIndex) const {
			if ( dwIndex > m_dwMaxIndex )
			{
				return false;
			}

			unsigned long dwTmpValue = *(unsigned long*)(m_pDataPtr+(dwIndex/8)*3) >> (dwIndex%8*3 );
			dwTmpValue = dwTmpValue & 0x7;
			return dwTmpValue;
		}

		bool set(const unsigned long dwIndex, const unsigned char ucVal){
			int  iIndexBitPos;
			static unsigned long caUinFlag[] = {0xFFFFFFF8, 0xFFFFFFC7, 0xFFFFFE3F, 0xFFFFF1FF, 0xFFFF8FFF, 0xFFFC7FFF, 0xFFE3FFFF, 0xFF1FFFFF};
			unsigned long dwFlag = ucVal;

			if ( dwIndex > m_dwMaxIndex )
			{
				return false;
			}

			iIndexBitPos = dwIndex%8;

			dwFlag = dwFlag << (iIndexBitPos*3);

			*(unsigned long*)(m_pDataPtr+(dwIndex/8)*3) &= caUinFlag[iIndexBitPos];		//清位为0
			*(unsigned long*)(m_pDataPtr+(dwIndex/8)*3) |= dwFlag;//

			return true;
		}
		void clear()	{	memset(m_pDataPtr, 0, shm_size()-sizeof(SHead));	}
		const char* data() const {	return (char*)m_pDataPtr;	}
		size_t size() const {	return m_dwMaxIndex;	}
		size_t shm_size() const {	return m_pHead->dwShmSize;	}
	protected:
	private:
		SHead* m_pHead;
		char* m_pDataPtr;
		unsigned long m_dwMaxIndex;
		CShm m_oShm;
		char m_szErrMsg[1024];

	};


	//0-15
	template<> class CBitmap<4>
	{
		typedef size_t size_type;
		struct SHead{
			size_type dwShmSize;
		};

	public:
		CBitmap(){}
		~CBitmap(){}

		bool init(const int iShmKey, const unsigned long dwBitmapSize, const bool bCreate=true, const bool bReadOnly=false){
			size_type dwShmSize = ((dwBitmapSize+7)/8)*4 + sizeof(SHead);
			m_dwMaxIndex = dwBitmapSize;

			//create shm
			if ( m_oShm.getShmID() <= 0 || !m_oShm.attach() )
			{
				if ( !m_oShm.create(iShmKey, dwShmSize, bCreate, bReadOnly) )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm init create error: %s", m_oShm.getErrMsg());
					return false;
				}
			}

			char* pShmBuf = reinterpret_cast<char*>(m_oShm.getShmBuf());
			if ( m_oShm.isCreate() )
			{
				//shm head
				m_pHead = (SHead*)pShmBuf;

				m_pHead->dwShmSize = dwShmSize;

				if ( dwShmSize <= sizeof(SHead) )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm max size<%lu> is small.", dwShmSize);
					return false;
				}
				m_pDataPtr = (char*)((char*)pShmBuf+sizeof(SHead));
			}
			else
			{
				m_pHead = (SHead*)m_oShm.getShmBuf();
				m_pDataPtr = (char*)((char*)pShmBuf+sizeof(SHead));
			}

			return true;
		}

		int get(const unsigned long dwIndex) const {
			if ( dwIndex > m_dwMaxIndex )
			{
				return false;
			}

			unsigned char ucFlag;
			ucFlag = *(m_pDataPtr+dwIndex/ 2) >> (dwIndex%2*4);
			ucFlag = ucFlag & 0x0F;
			return ucFlag;
		}

		bool set(const unsigned long dwIndex, const unsigned char ucVal){
			int  iIndexBitPos;
			static unsigned char caUinFlag[] = {0xf0, 0x0f};
			unsigned long ucFlag = ucVal;

			if ( dwIndex > m_dwMaxIndex )
			{
				return false;
			}

			iIndexBitPos = dwIndex%2;


			ucFlag = ucFlag << (iIndexBitPos*4);

			*(m_pDataPtr+dwIndex/2) &= caUinFlag[iIndexBitPos];		//清位为0
			*(m_pDataPtr+dwIndex/2) |= ucFlag;//

			return true;
		}
		void clear()	{	memset(m_pDataPtr, 0, shm_size()-sizeof(SHead));	}
		const char* data() const {	return (char*)m_pDataPtr;	}
		size_t size() const {	return m_dwMaxIndex;	}
		size_t shm_size() const {	return m_pHead->dwShmSize;	}
	protected:
	private:
		SHead* m_pHead;
		char* m_pDataPtr;
		unsigned long m_dwMaxIndex;
		CShm m_oShm;
		char m_szErrMsg[1024];

	};


	//0-31
	template<> class CBitmap<5>
	{
		typedef size_t size_type;
		struct SHead{
			size_type dwShmSize;
		};

	public:
		CBitmap(){}
		~CBitmap(){}

		bool init(const int iShmKey, const unsigned long dwBitmapSize, const bool bCreate=true, const bool bReadOnly=false){
			size_type dwShmSize = ((dwBitmapSize+7)/8)*5 + sizeof(SHead);
			m_dwMaxIndex = dwBitmapSize;

			//create shm
			if ( m_oShm.getShmID() <= 0 || !m_oShm.attach() )
			{
				if ( !m_oShm.create(iShmKey, dwShmSize, bCreate, bReadOnly) )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm init create error: %s", m_oShm.getErrMsg());
					return false;
				}
			}

			char* pShmBuf = reinterpret_cast<char*>(m_oShm.getShmBuf());
			if ( m_oShm.isCreate() )
			{
				//shm head
				m_pHead = (SHead*)pShmBuf;

				m_pHead->dwShmSize = dwShmSize;

				if ( dwShmSize <= sizeof(SHead) )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm max size<%lu> is small.", dwShmSize);
					return false;
				}
				m_pDataPtr = (char*)((char*)pShmBuf+sizeof(SHead));
			}
			else
			{
				m_pHead = (SHead*)m_oShm.getShmBuf();
				m_pDataPtr = (char*)((char*)pShmBuf+sizeof(SHead));
			}

			return true;
		}

		int get(const unsigned long dwIndex) const {
			if ( dwIndex > m_dwMaxIndex )
			{
				return false;
			}

			uint64_t ui64TmpValue= *(int64_t*)(m_pDataPtr+(dwIndex/8)*5) >> (dwIndex%8*5 );
			ui64TmpValue = ui64TmpValue & 0x1F;
			return (int)ui64TmpValue;
		}

		bool set(const unsigned long dwIndex, const unsigned char ucVal){
			int  iIndexBitPos;
			static uint64_t caUinFlag[] = {0xFFFFFFFFFFFFFFE0LL, 0xFFFFFFFFFFFFFC1FLL, 0xFFFFFFFFFFFF83FFLL, 0xFFFFFFFFFFF07FFFLL, 0xFFFFFFFFFE0FFFFFLL, 0xFFFFFFFFC1FFFFFFLL, 0xFFFFFFF83FFFFFFFLL, 0xFFFFFF07FFFFFFFFLL};
			uint64_t ui64Flag = ucVal;

			if ( dwIndex > m_dwMaxIndex )
			{
				return false;
			}

			iIndexBitPos = dwIndex%8;

			ui64Flag = ui64Flag << (iIndexBitPos*5);

			*(uint64_t*)(m_pDataPtr+(dwIndex/8)*5) &= caUinFlag[iIndexBitPos];		//清位为0
			*(uint64_t*)(m_pDataPtr+(dwIndex/8)*5) |= ui64Flag;//

			return true;
		}
		void clear()	{	memset(m_pDataPtr, 0, shm_size()-sizeof(SHead));	}
		const char* data() const {	return (char*)m_pDataPtr;	}
		size_t size() const {	return m_dwMaxIndex;	}
		size_t shm_size() const {	return m_pHead->dwShmSize;	}
	protected:
	private:
		SHead* m_pHead;
		char* m_pDataPtr;
		unsigned long m_dwMaxIndex;
		CShm m_oShm;
		char m_szErrMsg[1024];

	};




	//0-63
	template<> class CBitmap<6>
	{
		typedef size_t size_type;
		struct SHead{
			size_type dwShmSize;
		};

	public:
		CBitmap(){}
		~CBitmap(){}

		bool init(const int iShmKey, const unsigned long dwBitmapSize, const bool bCreate=true, const bool bReadOnly=false){
			size_type dwShmSize = ((dwBitmapSize+7)/8)*6 + sizeof(SHead);

			m_dwMaxIndex = dwBitmapSize;
			//create shm
			if ( m_oShm.getShmID() <= 0 || !m_oShm.attach() )
			{
				if ( !m_oShm.create(iShmKey, dwShmSize, bCreate, bReadOnly) )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm init create error: %s", m_oShm.getErrMsg());
					return false;
				}
			}

			char* pShmBuf = reinterpret_cast<char*>(m_oShm.getShmBuf());
			if ( m_oShm.isCreate() )
			{
				//shm head
				m_pHead = (SHead*)pShmBuf;

				m_pHead->dwShmSize = dwShmSize;

				if ( dwShmSize <= sizeof(SHead) )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm max size<%lu> is small.", dwShmSize);
					return false;
				}
				m_pDataPtr = (char*)((char*)pShmBuf+sizeof(SHead));
			}
			else
			{
				m_pHead = (SHead*)m_oShm.getShmBuf();
				m_pDataPtr = (char*)((char*)pShmBuf+sizeof(SHead));
			}

			return true;
		}

		int get(const unsigned long dwIndex) const {
			if ( dwIndex > m_dwMaxIndex )
			{
				return false;
			}

			unsigned long dwTmpValue = *(unsigned long*)(m_pDataPtr+(dwIndex/4)*3) >> (dwIndex%4*6 );
			dwTmpValue = dwTmpValue & 0x3F;
			return dwTmpValue;
		}

		bool set(const unsigned long dwIndex, const unsigned char ucVal){
			int  iIndexBitPos;
			static unsigned long caUinFlag[] = {0xFFFFFFC0, 0xFFFFF03F, 0xFFFC0FFF, 0xFF03FFFF};
			unsigned long dwFlag = ucVal;

			if ( dwIndex > m_dwMaxIndex )
			{
				return false;
			}

			iIndexBitPos = dwIndex%4;

			dwFlag = dwFlag << (iIndexBitPos*6);

			*(unsigned long*)(m_pDataPtr+(dwIndex/4)*3) &= caUinFlag[iIndexBitPos];		//清位为0
			*(unsigned long*)(m_pDataPtr+(dwIndex/4)*3) |= dwFlag;//

			return true;
		}
		void clear()	{	memset(m_pDataPtr, 0, shm_size()-sizeof(SHead));	}
		const char* data() const {	return (char*)m_pDataPtr;	}
		size_t size() const {	return m_dwMaxIndex;	}
		size_t shm_size() const {	return m_pHead->dwShmSize;	}
	protected:
	private:
		SHead* m_pHead;
		char* m_pDataPtr;
		unsigned long m_dwMaxIndex;
		CShm m_oShm;
		char m_szErrMsg[1024];

	};




	template<> class CBitmap<8>
	{
		typedef size_t size_type;
		struct SHead{
			size_type dwShmSize;
		};

	public:
		CBitmap(){}
		~CBitmap(){}

		bool init(const int iShmKey, const unsigned long dwBitmapSize, const bool bCreate=true, const bool bReadOnly=false){
			size_type dwShmSize = dwBitmapSize + sizeof(SHead);
			m_dwMaxIndex = dwBitmapSize;

			//create shm
			if ( m_oShm.getShmID() <= 0 || !m_oShm.attach() )
			{
				if ( !m_oShm.create(iShmKey, dwShmSize, bCreate, bReadOnly) )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm init create error: %s", m_oShm.getErrMsg());
					return false;
				}
			}

			char* pShmBuf = reinterpret_cast<char*>(m_oShm.getShmBuf());
			if ( m_oShm.isCreate() )
			{
				//shm head
				m_pHead = (SHead*)pShmBuf;

				m_pHead->dwShmSize = dwShmSize;

				if ( dwShmSize <= sizeof(SHead) )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm max size<%lu> is small.", dwShmSize);
					return false;
				}
				m_pDataPtr = (unsigned char*)((char*)pShmBuf+sizeof(SHead));
			}
			else
			{
				m_pHead = (SHead*)m_oShm.getShmBuf();
				m_pDataPtr = (unsigned char*)((char*)pShmBuf+sizeof(SHead));
			}

			return true;
		}

		int get(const unsigned long dwIndex) const {
			if ( dwIndex > m_dwMaxIndex )
			{
				return false;
			}

			return *(m_pDataPtr+dwIndex);
		}

		bool set(const unsigned long dwIndex, const unsigned char ucVal){
			if ( dwIndex > m_dwMaxIndex )
			{
				return false;
			}

			*(m_pDataPtr+dwIndex) = ucVal;
			return true;
		}
		void clear()	{	memset(m_pDataPtr, 0, shm_size()-sizeof(SHead));	}
		const char* data() const {	return (char*)m_pDataPtr;	}
		size_t size() const {	return m_dwMaxIndex;	}
		size_t shm_size() const {	return m_pHead->dwShmSize;	}
	protected:
	private:
		SHead* m_pHead;
		unsigned char* m_pDataPtr;
		unsigned long m_dwMaxIndex;
		CShm m_oShm;
		char m_szErrMsg[1024];

	};



	typedef CBitmap<1> BITMAP;	//1bit的bitmap

	//1bit的bitmap
	typedef CBitmap<1> BITMAP1;

	typedef CBitmap<2> BITMAP2;	//2bit的bitmap

	typedef CBitmap<3> BITMAP3;	//3bit的bitmap

	typedef CBitmap<4> BITMAP4;	//4bit的bitmap

	typedef CBitmap<5> BITMAP5;	//5bit的bitmap

	typedef CBitmap<6> BITMAP6;	//6bit的bitmap

	typedef CBitmap<8> BITMAP8;	//8bit的bitmap


};

#endif


