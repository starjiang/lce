#ifndef __NCE_PACKAGE_H__
#define __NCE_PACKAGE_H__

#include <stdexcept>
#include <string>
#include <memory.h>
#include "Utils.h"

namespace lce
{

template<typename T>
class CPackage
{
public:
	typedef std::runtime_error Error;
	typedef CPackage this_type;
	typedef T PKG_HEAD;
public:
	enum SEEK_DIR
	{
		S_BEG,
		S_CUR,
		S_END
	};

	CPackage(void)
	{
		m_iPos = 0;
		m_pkgData.assign(sizeof(PKG_HEAD), 0);
	}

	CPackage(const char* pszPkgData,const int iLen)
	{
		if (NULL == pszPkgData)
		{
			throw Error("pszPkgData is null.");
		}
		m_iPos = 0;
		m_pkgData.erase();
		m_pkgData.assign((char*)pszPkgData,iLen);
	}

	~CPackage(void)
	{
	}

	CPackage(const CPackage& rhs)
	{
		this->assign(rhs);
	}

	CPackage& operator= (const CPackage& rhs)
	{
		if (this != &rhs)
		{
			this->assign();
		}

		return *this;
	}

	void setPkgData(const unsigned char* pszPkgData,const int iLen)
	{
		this->setPkgData(reinterpret_cast<const char*>(pszPkgData), iLen);
	}
	void setPkgData(const char* pszPkgData,const int iLen)
	{
		if (NULL == pszPkgData)
		{
			throw Error("pszPkgData is null.");
		}
		else if ( static_cast<int>(sizeof(PKG_HEAD)) > iLen )
		{
			throw Error("data len less than pkg head len.");
		}
		m_iPos = 0;
		m_pkgData.erase();
		m_pkgData.assign(pszPkgData,iLen);
	}


	void setPkgData(const std::string sData)
	{
		m_iPos = 0;
		m_pkgData = sData;
	}

	//写数据
	void write(const unsigned char* pszData,const unsigned short wLen)
	{
		m_pkgData.append(reinterpret_cast<const char*>(pszData), wLen);
	}
	void write(const char* pszData,const unsigned short wLen)
	{
		m_pkgData.append(pszData,wLen);
	}

	size_t write(const unsigned long dwVal)
	{
		size_t dwPos = m_pkgData.size();
		unsigned long dwTmp = htonl(dwVal);
		m_pkgData.append(reinterpret_cast<unsigned char*>(&dwTmp),sizeof(unsigned long));
		return dwPos;
	}

	size_t write(const unsigned short wVal)
	{
		size_t dwPos = m_pkgData.size();
		unsigned short wTmp = htons(wVal) ;
		m_pkgData.append(reinterpret_cast<const char*>(&wTmp),sizeof(unsigned short));
		return dwPos;
	}

	size_t write(const unsigned char ucVal)
	{
		size_t dwPos = m_pkgData.size();
		m_pkgData.append(reinterpret_cast<const char*>(&ucVal),sizeof(unsigned char));
		return dwPos;
	}

	void writePos(const unsigned long dwVal, const size_t dwPos){
		unsigned long dwTmp = htonl(dwVal) ;
		m_pkgData.replace(dwPos, sizeof(unsigned long), reinterpret_cast<char*>(&dwTmp),sizeof(unsigned long));
	}

	void writePos(const unsigned short wVal, const size_t dwPos){
		unsigned short wTmp = htons(wVal) ;
		m_pkgData.replace(dwPos, sizeof(unsigned short), reinterpret_cast<char*>(&wTmp),sizeof(unsigned short));
	}

	void writePos(const unsigned char ucVal, const size_t dwPos){
		m_pkgData.replace(dwPos, sizeof(unsigned char), reinterpret_cast<const char*>(&ucVal),sizeof(unsigned char));
	}

	this_type& operator << (const std::string& sVal)
	{
		m_pkgData += sVal;
		return *this;
	}
	this_type& operator << (const char* pszVal)
	{
		m_pkgData.append(pszVal, strlen(pszVal));
		return *this;
	}
	this_type& operator << (const unsigned short wVal)
	{
		unsigned short wTmp = htons(wVal) ;
		m_pkgData.append(reinterpret_cast<char*>(&wTmp),sizeof(unsigned short));
		return *this;
	}
	this_type& operator << (const short wVal)
	{
		short shTmp = htons(wVal) ;
		m_pkgData.append(reinterpret_cast<char*>(&shTmp),sizeof(short));
		return *this;
	}
	#ifdef __x86_64__

	this_type& operator << (const unsigned long dwVal)
	{
		unsigned long dwTmp = htonll(dwVal);
		m_pkgData.append(reinterpret_cast<char*>(&dwTmp),sizeof(unsigned long));
		return *this;
	}
	this_type& operator << (const long dwVal)
	{
		long dwTmp = htonll(dwVal);
		m_pkgData.append(reinterpret_cast<char*>(&dwTmp),sizeof(long));
		return *this;
	}
    #else

    this_type& operator << (const unsigned long dwVal)
	{
		unsigned long dwTmp = htonl(dwVal);
		m_pkgData.append(reinterpret_cast<char*>(&dwTmp),sizeof(unsigned long));
		return *this;
	}
	this_type& operator << (const long dwVal)
	{
		long dwTmp = htonl(dwVal);
		m_pkgData.append(reinterpret_cast<char*>(&dwTmp),sizeof(long));
		return *this;
	}
	#endif

	this_type& operator << (const int iVal)
	{
		int iTmp = htonl(iVal);
		m_pkgData.append(reinterpret_cast<char*>(&iTmp),sizeof(int));
		return *this;
	}
	this_type& operator << (const unsigned int uiVal)
	{
		unsigned int uiTmp = htonl(uiVal);
		m_pkgData.append(reinterpret_cast<char*>(&uiTmp),sizeof(unsigned int));
		return *this;
	}
	template<typename TVal>
		this_type& operator << (const TVal& tVal)
	{
		m_pkgData.append(reinterpret_cast<const char*>(&tVal),sizeof(tVal));
		return *this;
	}

	//读数据
	template<typename TVal>
	this_type& operator >> (TVal& tVal)
	{
		if (sizeof(tVal) <= (m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
		{
			memcpy(&tVal,m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,sizeof(tVal));
		}
		else
		{
			throw Error("data error:no enough buf.");
		}
		m_iPos += static_cast<int>(sizeof(tVal));
		return *this;
	}

	this_type& operator >> (unsigned short& wVal)
	{
		if (sizeof(unsigned short) <= (m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
		{
			memcpy(&wVal,m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,sizeof(wVal));
			wVal = ntohs(wVal);
		}
		else
		{
			throw Error("data error:no enough buf.");
		}
		m_iPos += static_cast<int>(sizeof(unsigned short));
		return *this;
	}
	this_type& operator >> (short& wVal)
	{
		if (sizeof(short) <= (m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
		{
			memcpy(&wVal,m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,sizeof(wVal));
			wVal = ntohs(wVal);
		}
		else
		{
			throw Error("data error:no enough buf.");
		}
		m_iPos += static_cast<int>(sizeof(short));
		return *this;
	}
	this_type& operator >> (unsigned long& dwVal)
	{
		if (sizeof(unsigned long) <= (m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
		{
			memcpy(&dwVal,m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,sizeof(dwVal));
			dwVal = ntohl(dwVal);
		}
		else
		{
			throw Error("data error:no enough buf.");
		}
		m_iPos += static_cast<int>(sizeof(unsigned long));
		return *this;
	}
	this_type& operator >> (long& dwVal)
	{
		if (sizeof(long) <= (m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
		{
			memcpy(&dwVal,m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,sizeof(dwVal));
			dwVal = ntohl(dwVal);
		}
		else
		{
			throw Error("data error:no enough buf.");
		}
		m_iPos += static_cast<int>(sizeof(long));
		return *this;
	}
	this_type& operator >> (unsigned int& uiVal)
	{
		if (sizeof(unsigned int) <= (m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
		{
			memcpy(&uiVal,m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,sizeof(uiVal));
			uiVal = ntohl(uiVal);
		}
		else
		{
			throw Error("data error:no enough buf.");
		}
		m_iPos += static_cast<int>(sizeof(unsigned int));
		return *this;
	}
	this_type& operator >> (int& iVal)
	{
		if (sizeof(int) <= (m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
		{
			memcpy(&iVal,m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,sizeof(iVal));
			iVal = ntohl(iVal);
		}
		else
		{
			throw Error("data error:no enough buf.");
		}
		m_iPos += static_cast<int>(sizeof(int));
		return *this;
	}


	void read(char* pszBuf,const int iReadLen)
	{
		if (iReadLen <= static_cast<int>(m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
		{
			memcpy(pszBuf,m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,iReadLen);
		}
		else
		{
			throw Error("data error:no enough buf.");
		}
		m_iPos += iReadLen;
	}

	void read(std::string& sBuf, const int iReadLen)
	{
		if (iReadLen <= static_cast<int>(m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
		{
			sBuf.assign(m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,iReadLen);
		}
		else
		{
			throw Error("data error:no enough buf.");
		}
		m_iPos += iReadLen;
	}

	void readString(std::string& sData){
		const char* pData = m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos;
		int iStrLen = strlen(pData);
		sData = pData;
		m_iPos += iStrLen + 1;
	}

	void readString1(std::string& sData){
		if ( 1 <= static_cast<int>(m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
		{
			unsigned char ucLen = *(unsigned char*)(m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos);
			++m_iPos;
			read(sData, ucLen);
		}
		else
		{
			throw Error("data error:no enough buf.");
		}

	}

	void readString2(std::string& sData){
		if ( 2 <= static_cast<int>(m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
		{
			unsigned short wLen = ntohs(*(unsigned short*)(m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos));
			m_iPos += 2;
			read(sData, wLen);
		}
		else
		{
			throw Error("data error:no enough buf.");
		}

	}

	void readString4(std::string& sData){
		unsigned char dwLen = 0;
		if ( 4 <= static_cast<int>(m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
		{
			unsigned long dwLen = ntohl(*(unsigned long*)(m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos));
			m_iPos += 4;
			read(sData, dwLen);
		}
		else
		{
			throw Error("data error:no enough buf.");
		}
	}

	bool isEnd(const int iReadNum=0)const {
		return m_pkgData.size() >= m_iPos + sizeof(PKG_HEAD)+iReadNum  ? false : true;
	}

	//移动读指针位置
	bool seek(const int iOffPos,const SEEK_DIR pos = S_CUR)
	{
		bool bOk = false;
		switch(pos) {
		case S_BEG:
			if (iOffPos>=0 && static_cast<int>(m_pkgData.size()-sizeof(PKG_HEAD)) > iOffPos)
			{
				m_iPos = iOffPos;
				bOk = true;
			}
			break;
		case S_CUR:
			if ( static_cast<int>(m_pkgData.size()-sizeof(PKG_HEAD)) > iOffPos && m_iPos+iOffPos>=0)
			{
				m_iPos += iOffPos;
				bOk = true;
			}
			break;
		case S_END:
			if (iOffPos<=0 && (int)m_pkgData.size()>abs(iOffPos))
			{

				m_iPos = (int)m_pkgData.size()-sizeof(PKG_HEAD) + iOffPos;
				bOk = true;
			}
			break;
		}

		return bOk;
	}
	//清除数据
	void clear()
	{
		m_iPos = 0;
		m_pkgData.assign(sizeof(PKG_HEAD), 0);
	}

	//清除包体
	void clearBody()
	{
		m_iPos = 0;
		m_pkgData.erase(sizeof(PKG_HEAD));
	}

	//设置包体
	void setPkgBody(const unsigned char* pszBody,const int iBodyLen)
	{
		m_pkgData.replace(sizeof(PKG_HEAD),std::string::npos,  reinterpret_cast<const char*>(pszBody),iBodyLen);
	}
	void setPkgBody(const char* pszBody,const int iBodyLen)
	{
		m_pkgData.replace(sizeof(PKG_HEAD),std::string::npos,  pszBody,iBodyLen);
	}

	//设置包头
	void setPkgHead(T& pkgHead)
	{
		memcpy(const_cast<char*>(m_pkgData.data()),&pkgHead,sizeof(PKG_HEAD));
	}
	//获取包指针
	const char* getPkg() const {	return m_pkgData.data();	}
	const char* data() const {	return m_pkgData.data();	}
	//获取包长度
	int getPkgLen() const {	return static_cast<int>(m_pkgData.size());	}
	int size() const {	return static_cast<int>(m_pkgData.size()); }
	//获取包体指针
	const char* getPkgBody() const {	return m_pkgData.data()+sizeof(PKG_HEAD);	}
	//获取包体长度
	int getPkgBodyLen() const {	return static_cast<int>(m_pkgData.size()-sizeof(PKG_HEAD));	}
	//获取包头
	PKG_HEAD* getPkgHead() { return (reinterpret_cast<PKG_HEAD*>(const_cast<char*>(m_pkgData.data())));	}
	PKG_HEAD& head() {	return *(PKG_HEAD*)m_pkgData.data();	}
private:
	void assign(const this_type& rhs)
	{
		m_pkgData = rhs.m_pkgData;
		m_iPos = rhs.m_iPos;
	}
	int m_iPos;
	std::string m_pkgData;
};

};

#endif
