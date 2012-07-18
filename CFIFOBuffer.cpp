#include "CFIFOBuffer.h"
#include <cassert>

namespace lce
{


CFIFOBuffer::CFIFOBuffer(void)
:m_pszDataBuf(NULL)
,m_iBufferLen(0)
,m_iHeadPos(0)
,m_iTailPos(0)
,m_iCurDataLen(0)
,m_iResetCount(0)
,m_iReadCount(0)
,m_iWriteCount(0)
{
}

CFIFOBuffer::~CFIFOBuffer(void)
{
	delete m_pszDataBuf;
}

void CFIFOBuffer::reset()
{
	m_iHeadPos = 0;
	m_iTailPos = 0;
	m_iCurDataLen = 0;
	++m_iResetCount;
}

bool CFIFOBuffer::init(const int iBufLen)
{
	bool bOk = false;
	if ( iBufLen < MIN_BUF_LEN )
	{
		m_iBufferLen = MIN_BUF_LEN;
	}
	else
	{
		m_iBufferLen = iBufLen;
	}

	m_pszDataBuf = new unsigned char[m_iBufferLen];
	if (NULL != m_pszDataBuf)
	{
		bOk = true;
	}
	else
	{
		m_sErrMsg = "memory no enough.";
		bOk = false;
		m_iBufferLen = 0;
	}

	return bOk;
}


CFIFOBuffer::RETURN_TYPE CFIFOBuffer::readNext()
{
	CFIFOBuffer::RETURN_TYPE nRe = BUF_OK;

	assert(m_iHeadPos>=0 && m_iHeadPos<=m_iBufferLen);
	assert(m_iTailPos>=0 && m_iTailPos<=m_iBufferLen);

	int iTmpTailPos = m_iTailPos;

	if (m_iHeadPos < iTmpTailPos)
	{
		//判断数据长度的合法性, 如果非法就数据清空,重来
		if (m_iHeadPos+HEAD_LEN > iTmpTailPos)
		{
			//reset buffer
			char szErrMsg[1024];
			snprintf(szErrMsg, sizeof(szErrMsg),  "data error:(m_iHeadPos<%d>+4 > dwTailPos<%d>)(file:%s,line:%d)",m_iHeadPos,iTmpTailPos, __FILE__, __LINE__);
			m_sErrMsg = szErrMsg;
			this->reset();
			assert(0);
			return BUF_ERR;
		}

		//读取数据长度信息
		m_iCurDataLen = *(int *)(m_pszDataBuf+m_iHeadPos);

		//判断数据长度的合法性, 如果非法把数据清空,重来
		if (m_iHeadPos+HEAD_LEN+m_iCurDataLen > iTmpTailPos)
		{
			char szErrMsg[1024];
			snprintf(szErrMsg, sizeof(szErrMsg),  "data error:(m_iHeadPos<%d>+sizeof(SDataHead)<%d>+4 > dwTailPos<%d>)(file:%s,line:%d).", m_iHeadPos, m_iCurDataLen, iTmpTailPos, __FILE__, __LINE__);
			m_sErrMsg = szErrMsg;
			this->reset();
			assert(0);
			return BUF_ERR;
		}

		//读取数据内容信息
		m_iHeadPos+= HEAD_LEN;
	}
	else if (m_iHeadPos > iTmpTailPos)
	{
		if (m_iHeadPos+HEAD_LEN <= m_iBufferLen)
		{
			//读取数据头信息
			m_iCurDataLen = *(int *)(m_pszDataBuf+m_iHeadPos);
			if ( m_iCurDataLen + m_iHeadPos + HEAD_LEN > m_iBufferLen )
//			if(EMPTY_DATA == stDataHead.ucDataType)
			{//已到buffer尾部,从开始读数据
				m_iHeadPos = 0;
				//判断数据长度的合法性, 如果非法就数据清空,重来
				if (m_iHeadPos == iTmpTailPos)
				{
					char szErrMsg[1024];
					snprintf(szErrMsg, sizeof(szErrMsg),  "data error:(m_iHeadPos<%d>+4 > dwTailPos<%d>)(file:%s,line:%d)",m_iHeadPos,iTmpTailPos, __FILE__, __LINE__);
					m_sErrMsg = szErrMsg;
					this->reset();
					assert(0);
					return BUF_ERR;
				}
				else if (m_iHeadPos+HEAD_LEN > iTmpTailPos)
				{
					//reset buffer
					char szErrMsg[1024];
					snprintf(szErrMsg, sizeof(szErrMsg), "data error:(m_iHeadPos<%d>+4 > dwTailPos<%d>)(file:%s,line:%d)",m_iHeadPos,iTmpTailPos, __FILE__, __LINE__);
					m_sErrMsg = szErrMsg;
					this->reset();
					assert(0);
					return BUF_ERR;
				}

				//判断数据长度的合法性, 如果非法就数据清空,重来
				if (m_iHeadPos+m_iCurDataLen > iTmpTailPos)
				{
					char szErrMsg[1024];
					snprintf(szErrMsg, sizeof(szErrMsg), "data error:(m_iHeadPos<%d>+DataLen<%d> > dwTailPos<%d>)(file:%s,line:%d).",m_iHeadPos, m_iCurDataLen, iTmpTailPos, __FILE__, __LINE__);
					m_sErrMsg = szErrMsg;
					this->reset();
					assert(0);
					return BUF_ERR;
				}
			}
			else
			{
				//判断数据长度的合法性, 如果非法就数据清空,重来
				if (m_iHeadPos+HEAD_LEN+m_iCurDataLen > m_iBufferLen)
				{
					char szErrMsg[1024];
					snprintf(szErrMsg, sizeof(szErrMsg), "data error:<dwTailPos=%d>(m_iHeadPos<%d>+4+wDataLen<%d> > m_iBufferLen<%d>)(file:%s,line:%d).",iTmpTailPos, m_iHeadPos,m_iCurDataLen,m_iBufferLen, __FILE__, __LINE__);
					m_sErrMsg = szErrMsg;
					this->reset();
					assert(0);
					return BUF_ERR;
				}

				//读取数据内容信息
				m_iHeadPos += HEAD_LEN ;
			}
		}
		else //已到buffer尾部,从开始读数据
		{
			m_iHeadPos = 0;
			//判断数据长度的合法性, 如果非法就数据清空,重来
			if (m_iHeadPos == iTmpTailPos)
			{
				return BUF_EMPTY;
			}
			else if (m_iHeadPos+HEAD_LEN > iTmpTailPos)
			{
				//reset buffer
				char szErrMsg[1024];
				snprintf(szErrMsg, sizeof(szErrMsg), "data error:(m_iHeadPos<%d>+4> dwTailPos<%d>)(file:%s,line:%d)",m_iHeadPos, iTmpTailPos, __FILE__, __LINE__);
				m_sErrMsg = szErrMsg;
				this->reset();
				assert(0);
				return BUF_ERR;
			}

			//读取数据头信息
			m_iCurDataLen = *(int *)(m_pszDataBuf+m_iHeadPos);

			//判断数据长度的合法性, 如果非法就数据清空,重来
			if (m_iHeadPos+HEAD_LEN+m_iCurDataLen > iTmpTailPos)
			{
				char szErrMsg[1024];
				snprintf(szErrMsg, sizeof(szErrMsg), "data error:(m_iHeadPos<%d>+4+wDataLen<%d> > dwTailPos<%d>)(file:%s,line:%d).",m_iHeadPos, m_iCurDataLen, iTmpTailPos, __FILE__, __LINE__);
				m_sErrMsg = szErrMsg;
				this->reset();
				assert(0);
				return BUF_ERR;
			}

			//读取数据内容信息
			m_iHeadPos += HEAD_LEN;
		}
	}
	else//if (m_iHeadPos == dwTailPos)
	{
		//没有数据
		nRe = BUF_EMPTY;
	}

	if ( nRe == BUF_OK )
	{
		++m_iReadCount;
	}

	return nRe;
}



CFIFOBuffer::RETURN_TYPE CFIFOBuffer::read(unsigned char* pszBuf, int& iBufLen)
{
	CFIFOBuffer::RETURN_TYPE nRe = readNext();
	if ( CFIFOBuffer::BUF_OK == nRe )
	{
		if ( getCurDataLen() <= iBufLen )
		{
			memcpy(pszBuf, getCurData(), getCurDataLen());
			iBufLen=getCurDataLen();
		}
		else
		{
			nRe = CFIFOBuffer::BUF_NO_ENOUGH;
		}
		moveNext();
	}

	return nRe;
}

CFIFOBuffer::RETURN_TYPE CFIFOBuffer::read(std::string& sBuf)
{
	CFIFOBuffer::RETURN_TYPE nRe = readNext();
	if ( CFIFOBuffer::BUF_OK == nRe )
	{
		sBuf.assign(reinterpret_cast<const char*>(getCurData()), getCurDataLen());
		moveNext();
	}
	return nRe;
}



CFIFOBuffer::RETURN_TYPE CFIFOBuffer::write(const unsigned char* pszBuf, const int iBufLen)
{

	assert(m_iHeadPos>=0 && m_iHeadPos<=m_iBufferLen);
	assert(m_iTailPos>=0 && m_iTailPos<=m_iBufferLen);

	int iTmpHeadPos = m_iHeadPos;

	CFIFOBuffer::RETURN_TYPE nRe = BUF_OK;

	//获取保存数据头和数据内容的位置
	if (iTmpHeadPos <= m_iTailPos)
	{
		if (m_iTailPos+HEAD_LEN+iBufLen <= m_iBufferLen)
		{
			//可写
			//保存数据头和数据内容
			memcpy(m_pszDataBuf+m_iTailPos, &iBufLen, HEAD_LEN);
			memcpy(m_pszDataBuf+m_iTailPos+HEAD_LEN, pszBuf, iBufLen);
			m_iTailPos += HEAD_LEN+iBufLen;
		}
		else
		{
			//已到buffer的尾部,只能保存数据头
			if (m_iTailPos+HEAD_LEN <= m_iBufferLen)
			{
				if (iBufLen < iTmpHeadPos )
				{
					memcpy(m_pszDataBuf+m_iTailPos, &iBufLen, HEAD_LEN);
					memcpy(m_pszDataBuf, pszBuf, iBufLen);
					m_iTailPos = iBufLen;
				}
				else
				{
					nRe = BUF_FULL;
				}
			}
			else
			{
				if (iBufLen+HEAD_LEN < iTmpHeadPos)
				{
					memcpy(m_pszDataBuf, &iBufLen, HEAD_LEN);
					memcpy(m_pszDataBuf+HEAD_LEN, pszBuf, iBufLen);
					m_iTailPos = HEAD_LEN+iBufLen;
				}
				else
				{
					nRe = BUF_FULL;
				}
			}
		}
	}
	else
	{
		if (m_iTailPos+HEAD_LEN+iBufLen < iTmpHeadPos)
		{
			//可写
			//保存数据头和数据内容
			memcpy(m_pszDataBuf+m_iTailPos, &iBufLen, HEAD_LEN);
			memcpy(m_pszDataBuf+m_iTailPos+HEAD_LEN, pszBuf, iBufLen);
			m_iTailPos += HEAD_LEN+iBufLen;
		}
		else //buffer已满
		{
			nRe = BUF_FULL;
		}
	}

	if ( nRe == BUF_OK )
	{
		++m_iWriteCount;
	}


	return nRe;
}

CFIFOBuffer::RETURN_TYPE CFIFOBuffer::write(const unsigned char* pszData1, const int iDataSize1, const unsigned char* pszData2, const int iDataSize2)
{

	assert(m_iHeadPos>=0 && m_iHeadPos<=m_iBufferLen);
	assert(m_iTailPos>=0 && m_iTailPos<=m_iBufferLen);

	int iTmpHeadPos = m_iHeadPos;

	CFIFOBuffer::RETURN_TYPE nRe = BUF_OK;
	int iBufLen = iDataSize1+iDataSize2;

	//获取保存数据头和数据内容的位置
	if (iTmpHeadPos <= m_iTailPos)
	{
		if (m_iTailPos+HEAD_LEN+iBufLen <= m_iBufferLen)
		{
			//可写
			//保存数据头和数据内容
			memcpy(m_pszDataBuf+m_iTailPos, &iBufLen, HEAD_LEN);
			memcpy(m_pszDataBuf+m_iTailPos+HEAD_LEN, pszData1, iDataSize1);
			memcpy(m_pszDataBuf+m_iTailPos+HEAD_LEN+iDataSize1, pszData2, iDataSize2);
			m_iTailPos += HEAD_LEN+iBufLen;
		}
		else
		{
			//已到buffer的尾部,只能保存数据头
			if (m_iTailPos+HEAD_LEN <= m_iBufferLen)
			{
				if (iBufLen < iTmpHeadPos )
				{
					memcpy(m_pszDataBuf+m_iTailPos, &iBufLen, HEAD_LEN);
					memcpy(m_pszDataBuf, pszData1, iDataSize1);
					memcpy(m_pszDataBuf+iDataSize1, pszData2, iDataSize2);
					m_iTailPos = iBufLen;
				}
				else
				{
					nRe = BUF_FULL;
				}
			}
			else
			{
				if ( HEAD_LEN+iBufLen < iTmpHeadPos )
				{
					memcpy(m_pszDataBuf, &iBufLen, HEAD_LEN);
					memcpy(m_pszDataBuf+HEAD_LEN, pszData1, iDataSize1);
					memcpy(m_pszDataBuf+HEAD_LEN+iDataSize1, pszData2, iDataSize2);
					m_iTailPos = HEAD_LEN+iBufLen;
				}
				else
				{
					nRe = BUF_FULL;
				}
			}
		}
	}
	else
	{
		if (m_iTailPos+HEAD_LEN+iBufLen < iTmpHeadPos)
		{
			//可写
			//保存数据头和数据内容
			memcpy(m_pszDataBuf+m_iTailPos, &iBufLen, HEAD_LEN);
			memcpy(m_pszDataBuf+m_iTailPos+HEAD_LEN, pszData1, iDataSize1);
			memcpy(m_pszDataBuf+m_iTailPos+HEAD_LEN+iDataSize1, pszData2, iDataSize2);
			m_iTailPos += HEAD_LEN+iBufLen;
		}
		else //buffer已满
		{
			nRe = BUF_FULL;
		}
	}

	if ( nRe == BUF_OK )
	{
		++m_iWriteCount;
	}


	return nRe;
}


CFIFOBuffer::RETURN_TYPE CFIFOBuffer::write(const unsigned char* pszData1, const int iDataSize1, const unsigned char* pszData2, const int iDataSize2, const unsigned char* pszData3, const int iDataSize3)
{

	assert(m_iHeadPos>=0 && m_iHeadPos<=m_iBufferLen);
	assert(m_iTailPos>=0 && m_iTailPos<=m_iBufferLen);

	int iTmpHeadPos = m_iHeadPos;

	CFIFOBuffer::RETURN_TYPE nRe = BUF_OK;
	int iBufLen = iDataSize1+iDataSize2+iDataSize3;

	//获取保存数据头和数据内容的位置
	if (iTmpHeadPos <= m_iTailPos)
	{
		if (m_iTailPos+HEAD_LEN+iBufLen <= m_iBufferLen)
		{
			//可写
			//保存数据头和数据内容
			memcpy(m_pszDataBuf+m_iTailPos, &iBufLen, HEAD_LEN);
			memcpy(m_pszDataBuf+m_iTailPos+HEAD_LEN, pszData1, iDataSize1);
			memcpy(m_pszDataBuf+m_iTailPos+HEAD_LEN+iDataSize1, pszData2, iDataSize2);
			memcpy(m_pszDataBuf+m_iTailPos+HEAD_LEN+iDataSize1+iDataSize2, pszData3, iDataSize3);
			m_iTailPos += HEAD_LEN+iBufLen;
		}
		else
		{
			//已到buffer的尾部,只能保存数据头
			if (m_iTailPos+HEAD_LEN <= m_iBufferLen)
			{
				if (iBufLen < iTmpHeadPos )
				{
					memcpy(m_pszDataBuf+m_iTailPos, &iBufLen, HEAD_LEN);
					memcpy(m_pszDataBuf, pszData1, iDataSize1);
					memcpy(m_pszDataBuf+iDataSize1, pszData2, iDataSize2);
					memcpy(m_pszDataBuf+iDataSize1+iDataSize2, pszData3, iDataSize3);
					m_iTailPos = iBufLen;
				}
				else
				{
					nRe = BUF_FULL;
				}
			}
			else
			{
				if ( HEAD_LEN+iBufLen < iTmpHeadPos )
				{
					memcpy(m_pszDataBuf, &iBufLen, HEAD_LEN);
					memcpy(m_pszDataBuf+HEAD_LEN, pszData1, iDataSize1);
					memcpy(m_pszDataBuf+HEAD_LEN+iDataSize1, pszData2, iDataSize2);
					memcpy(m_pszDataBuf+HEAD_LEN+iDataSize1+iDataSize2, pszData3, iDataSize3);
					m_iTailPos = HEAD_LEN+iBufLen;
				}
				else
				{
					nRe = BUF_FULL;
				}
			}
		}
	}
	else
	{
		if (m_iTailPos+HEAD_LEN+iBufLen < iTmpHeadPos)
		{
			//可写
			//保存数据头和数据内容
			memcpy(m_pszDataBuf+m_iTailPos, &iBufLen, HEAD_LEN);
			memcpy(m_pszDataBuf+m_iTailPos+HEAD_LEN, pszData1, iDataSize1);
			memcpy(m_pszDataBuf+m_iTailPos+HEAD_LEN+iDataSize1, pszData2, iDataSize2);
			memcpy(m_pszDataBuf+m_iTailPos+HEAD_LEN+iDataSize1+iDataSize2, pszData3, iDataSize3);
			m_iTailPos += HEAD_LEN+iBufLen;
		}
		else //buffer已满
		{
			nRe = BUF_FULL;
		}
	}

	if ( nRe == BUF_OK )
	{
		++m_iWriteCount;
	}


	return nRe;
}

};

