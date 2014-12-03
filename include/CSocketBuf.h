#ifndef __NCE_SOCKET_BUF_H__
#define __NCE_SOCKET_BUF_H__

#include <assert.h>
#include <string.h>
namespace lce
{

class CSocketBuf
{
public:
	CSocketBuf(const int iInitSize, const int iAllowMaxSize)
		:m_iDataBeginPos(0)
		,m_iDataEndPos(0)
		,m_iDataAllowMaxSize(iAllowMaxSize)
		,m_iInitSize(iInitSize)
	{
		m_pszBuf = new char[m_iInitSize];
		m_iBufSize = m_iInitSize;
	}

	CSocketBuf()
	:m_iDataBeginPos(0)
	,m_iDataEndPos(0)
	,m_iDataAllowMaxSize(0)
	,m_iInitSize(0)
	{
		m_pszBuf = NULL;
	}

	~CSocketBuf()
	{
		delete[] m_pszBuf;
		m_pszBuf = NULL;
	}

	CSocketBuf(const CSocketBuf& rhs)
		:m_iDataBeginPos(0)
		,m_iDataEndPos(0)
		,m_iDataAllowMaxSize(0)
		,m_iInitSize(0)
		,m_pszBuf(NULL)
	{
		this->assign(rhs);
	}

	CSocketBuf& operator=(const CSocketBuf& rhs)
	{
		if ( this != &rhs )
		{
			this->assign(rhs);
		}

		return *this;
	}

	const char* getData() const {	return m_pszBuf + m_iDataBeginPos;	}
	int getSize() const {	return (m_iDataEndPos - m_iDataBeginPos);	}
	int getRealFreeSize() const {	return m_iBufSize - getSize();	}
	int getBufSize() const {	return m_iBufSize;	}
	int getBufMaxSize() const {	return m_iDataAllowMaxSize;	}

	//三函数配套使用
	int getFreeSize()	const {		return m_iBufSize - m_iDataEndPos;	}
	char* getFreeBuf()	{	return m_pszBuf + m_iDataEndPos;	}

	bool addData(const int iSize)
	{
		bool bOk = true;
		if ( m_iBufSize < m_iDataEndPos+iSize )
		{
			assert(0);
			bOk = false;
		}
		m_iDataEndPos += iSize;
		return bOk;
	}

	void addFreeBuf()
	{
		if ( 0 != m_iDataBeginPos )
		{
			moveData();
		}
		else
		{
			incBuf();
		}
	}
	void eraseFreeBuf()
	{

	}

	bool addData(const char* pszData, const int iSize)
	{
		bool bOk = true;
		if ( m_iDataAllowMaxSize-getSize() >= iSize )
		{
			while ( getFreeSize() < iSize )
			{
				if ( 0 != m_iDataBeginPos )
				{
					moveData();
				}
				else
				{
					incBuf();
				}
			}

			if ( m_iBufSize >= m_iDataEndPos+iSize  )
			{
				memcpy(m_pszBuf+m_iDataEndPos, pszData, iSize);
				m_iDataEndPos += iSize;
			}
			else
			{
				bOk = false;
			}
		}
		else
		{
			bOk = false;
		}
		return bOk;
	}

	void removeData(const int iSize)
	{
		m_iDataBeginPos += iSize;
		if ( m_iDataBeginPos == m_iDataEndPos )
		{
			reSize(m_iInitSize);
			m_iDataBeginPos = 0;
			m_iDataEndPos = 0;
		}
	}

	void reset()
	{
		m_iDataBeginPos = 0;
		m_iDataEndPos = 0;
		reSize(m_iInitSize);
	}

private:
	void assign(const CSocketBuf& rhs)
	{

		if ( NULL != m_pszBuf )
		{
			delete[] m_pszBuf;
			m_pszBuf = NULL;
		}

		m_pszBuf = new char[rhs.m_iBufSize];
		memcpy(m_pszBuf, rhs.m_pszBuf, rhs.m_iBufSize);
		m_iBufSize = rhs.m_iBufSize;
		m_iDataBeginPos = rhs.m_iDataBeginPos;
		m_iDataEndPos = rhs.m_iDataEndPos;
		m_iDataAllowMaxSize = rhs.m_iDataAllowMaxSize;
		m_iInitSize = rhs.m_iInitSize;
	}

	void moveData()
	{
		assert(m_iDataEndPos>=m_iDataBeginPos);
		if (m_iDataEndPos != m_iDataBeginPos)
		{
			memmove(m_pszBuf, m_pszBuf+m_iDataBeginPos, m_iDataEndPos-m_iDataBeginPos);
			m_iDataEndPos -= m_iDataBeginPos;
			m_iDataBeginPos = 0;
		}
	}

	void incBuf()
	{
		if (m_iBufSize*2 < m_iDataAllowMaxSize )
		{
			reSize(m_iBufSize*2);
		}
		else
		{
			reSize(m_iDataAllowMaxSize);
		}
	}


	bool decBuf()
	{
		bool bOk = false;
		if (getSize() < m_iBufSize/2)
		{
			if (m_iBufSize/2 < m_iInitSize )
			{
				bOk = reSize(m_iInitSize);
			}
			else
			{
				bOk = reSize(m_iBufSize/2);
			}
		}
		return bOk;
	}

	bool reSize(int iSize)
	{
		bool bOk = false;

		if ( m_iDataBeginPos != 0 )
			moveData();

		if ( iSize == m_iBufSize )
			return true;

		char* pszNewBuf = new char[iSize];
		assert( getSize() < iSize );
		if ( NULL != pszNewBuf )
		{
			if ( iSize>=getSize() )
				memcpy(pszNewBuf, getData(), getSize());

			m_iBufSize = iSize;
			delete[] m_pszBuf;
			m_pszBuf = pszNewBuf;
			bOk = true;
		}

		return bOk;
	}

private:
	int m_iBufSize;

	int m_iDataBeginPos;
	int m_iDataEndPos;
	int m_iDataAllowMaxSize;
	int m_iInitSize;

	char* m_pszBuf;
};

};



#endif
