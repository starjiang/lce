#ifndef __NCE_FIFO_BUFFER_BY_HEAD_H__
#define __NCE_FIFO_BUFFER_BY_HEAD_H__

#include "CFIFOBuffer.h"

namespace lce
{

template < typename T >

class CFIFOBufferByHead
{
	typedef CFIFOBufferByHead this_type;
	typedef T HEAD;
public:
	CFIFOBufferByHead()
		:m_iId(0)
	{}

	~CFIFOBufferByHead(){}

	bool init(const int iId, const int iBufLen){
		bool bOk = false;
		bOk = m_oFIFOBuffer.init(iBufLen);
		if (bOk)
		{
			m_iId = iId;
			bOk = true;
		}
		return bOk;
	}
	int getId() const	{	return m_iId;	}

	const char* getErrMsg() const {	return m_oFIFOBuffer.getErrMsg();	}

	CFIFOBuffer::RETURN_TYPE read(HEAD& stHead, unsigned char* pszBuf, int& iBufLen){
		string sReadBuf;
		CFIFOBuffer::RETURN_TYPE nRe = m_oFIFOBuffer.read(sReadBuf);
		if ( nRe == CFIFOBuffer::BUF_OK )
		{
			if ( sReadBuf.size() >= sizeof(HEAD) )
			{
				memcpy(&stHead, sReadBuf.data(), sizeof(HEAD));
				memcpy(pszBuf, sReadBuf.data()+sizeof(HEAD), sReadBuf.size()-sizeof(HEAD));
			}
			else
			{
				nRe = CFIFOBuffer::BUF_ERR;
			}
		}
		return nRe;
	}
	CFIFOBuffer::RETURN_TYPE read(HEAD& stHead, string& sReadBuf)
	{
		CFIFOBuffer::RETURN_TYPE nRe = m_oFIFOBuffer.read(sReadBuf);
		if ( nRe == CFIFOBuffer::BUF_OK )
		{
			if ( sReadBuf.size() >= sizeof(HEAD) )
			{
				memcpy(&stHead, sReadBuf.data(), sizeof(HEAD));
				sReadBuf.erase(0, sizeof(HEAD));
			}
			else
			{
				nRe = CFIFOBuffer::BUF_ERR;
			}
		}
		return nRe;
	}

	CFIFOBuffer::RETURN_TYPE write(const HEAD& stHead, const unsigned char* pszBuf, const int iBufLen)
	{
		return this->write(stHead, reinterpret_cast<const char*>(pszBuf), iBufLen);
	}
	CFIFOBuffer::RETURN_TYPE write(const HEAD& stHead, const char* pszBuf, const int iBufLen)	{
		string sWriteBuf;
		sWriteBuf.assign(static_cast<char*>(&stHead), sizeof(stHead));
		sWriteBuf.append(pszBuf, iBufLen);
		return m_oFIFOBuffer.write(sWriteBuf.data(), sWriteBuf.size());
	}
	CFIFOBuffer::RETURN_TYPE write(const HEAD& stHead, const string& sBuf)	{
		return this->write(stHead, sBuf.data(), sBuf.size());
	}

private:

	CFIFOBuffer m_oFIFOBuffer;
	int m_iId;						//buffer ID

};

};

#endif
