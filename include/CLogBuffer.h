/*************************************************************************
    > File Name: CLogBuffer.h
    > Author: starjiang
    > Mail: starjiang@qq.com 
    > Created Time: Tue May 20 14:49:17 2014
 ************************************************************************/
#ifndef __LCE_LOG_BUFFER_H__
#define __LCE_LOG_BUFFER_H__

namespace lce
{

class CLogBuffer
{
public:	
	CLogBuffer()
	{
		m_pData = NULL;
		m_dwMaxSize = 0;
		m_dwSize = 0;
		m_bFull = false;
	}

	~CLogBuffer()
	{
		if(m_pData!=NULL)
			delete []m_pData;
	}

	bool init(size_t dwSize = 1024*1024)
	{
		if(m_pData == NULL)
		{
			m_pData = new char[dwSize];
			m_dwMaxSize = dwSize;
			m_dwSize = 0;
			m_bFull = false;
		}
			
		return true;
	}

	bool empty()
	{
		return (m_dwSize == 0);
	}

	bool full()
	{
		return m_bFull;
	}

	bool write(const char *data,size_t dwLen)
	{
		if(m_pData == NULL)
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"buffer is not init");
			return false;
		}

		if(m_dwSize+dwLen > m_dwMaxSize)
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"buffer is full now=%u,data=%u",(unsigned int)m_dwSize,(unsigned int)dwLen);
			m_bFull = true;
			return false;
		}

		memcpy(m_pData+m_dwSize,data,dwLen);
		m_dwSize+=dwLen;
		return true;
	}

	char *data()
	{
		return m_pData;
	}

	size_t size()
	{
		return m_dwSize;
	}

	bool reset()
	{
		m_bFull = false;
		m_dwSize = 0;
		return true;
	}

	char * getErrMsg()
	{
		return m_szErrMsg;
	}

private:
	size_t m_dwMaxSize;
	size_t m_dwSize;
	char *m_pData;
	bool m_bFull;
	char m_szErrMsg[1024];

};

}
#endif
