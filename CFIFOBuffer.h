#ifndef __NCE_FIFO_BUFFER_H__
#define __NCE_FIFO_BUFFER_H__

#include <string.h>
#include <stdio.h>
#include <string>
namespace lce
{

class CFIFOBuffer
{
	typedef CFIFOBuffer this_type;
	enum {MIN_BUF_LEN=10*1024};

	enum{HEAD_LEN=sizeof(int)};

public:
	enum RETURN_TYPE{
		BUF_OK=0,
		BUF_FULL=1,
		BUF_EMPTY=2,
		BUF_NO_ENOUGH=3,		//保存的buffer不够
		BUF_ERR=4,
	};


	CFIFOBuffer(void);
	~CFIFOBuffer(void);

	bool init(const int iBufLen);	//初始化
	const char* getErrMsg() const {	return m_sErrMsg.c_str();	}

	//直接可以读取
	RETURN_TYPE read(unsigned char* pszBuf, int& iBufLen);
	RETURN_TYPE read(std::string& sBuf);


	int getCurDataLen() const {		return m_iCurDataLen;	}
	const unsigned char* getCurData() const {	return (m_pszDataBuf+m_iHeadPos);	}
	RETURN_TYPE readNext();
	void moveNext()	{	m_iHeadPos += m_iCurDataLen;	}
	//////////////////////////////////////////////////////////////////////////

	RETURN_TYPE write(const unsigned char* pszData1, const int iDataSize1, const unsigned char* pszData2, const int iDataSize2);
	inline RETURN_TYPE write(const char* pszData1, const int iDataSize1, const char* pszData2, const int iDataSize2){
		return write(reinterpret_cast<const unsigned char*>(pszData1), iDataSize1, reinterpret_cast<const unsigned char*>(pszData2), iDataSize2);
	}

	RETURN_TYPE write(const unsigned char* pszData1, const int iDataSize1, const unsigned char* pszData2, const int iDataSize2, const unsigned char* pszData3, const int iDataSize3);
	inline RETURN_TYPE write(const char* pszData1, const int iDataSize1, const char* pszData2, const int iDataSize2, const char* pszData3, const int iDataSize3){
		return write(reinterpret_cast<const unsigned char*>(pszData1), iDataSize1, reinterpret_cast<const unsigned char*>(pszData2), iDataSize2, reinterpret_cast<const unsigned char*>(pszData3), iDataSize3);
	}

	RETURN_TYPE write(const unsigned char* pszBuf, const int iBufLen);
	inline RETURN_TYPE write(const char* pszBuf, const int iBufLen)	{		return write(reinterpret_cast<const unsigned char*>(pszBuf), iBufLen);	}
	inline RETURN_TYPE write(const std::string& sBuf)	{	return write(sBuf.data(), static_cast<int>(sBuf.size()));	}

	int getSize() const {	return m_iBufferLen;	}
private:
	void reset();	//清空数据
private:
	std::string m_sErrMsg;

	unsigned char* m_pszDataBuf;	//数据buffer指针
	int m_iBufferLen;	//整个buffer的长度

	volatile int m_iHeadPos;		//已保存数据的头部位置
	volatile int m_iTailPos;   		//已保存数据的尾部位置

	int m_iCurDataLen;		//当前数据长度

	int m_iResetCount;				//reset 次数
	int m_iReadCount;				//read 次数
	int m_iWriteCount;				//write 次数
};

};

#endif

