#ifndef __HTTPRESPONSE_H__
#define __HTTPRESPONSE_H__

#include <map>
#include <string>
#include <sstream>
#include "Utils.h"
#include <map>

using std::string;

namespace lce
{

class CHttpResponse
{
	typedef CHttpResponse this_type;
	struct SCookieInfo{
		SCookieInfo(){}
		string sValue;
		string sDomain;
		string sPath;
		time_t dwCookieTime;
		SCookieInfo& operator=(const SCookieInfo& rhs){
			if (&rhs != this)
				assign(rhs);

			return *this;
		}
		SCookieInfo(const SCookieInfo& rhs)	{	assign(rhs);	}
	private:
		void assign(const SCookieInfo& rhs){
			sValue = rhs.sValue;
			sDomain = rhs.sDomain;
			sPath = rhs.sPath;
			dwCookieTime = rhs.dwCookieTime;
		}
	};
	typedef std::map<string, SCookieInfo> MAP_COOKIE;

	typedef std::map<string,string > MAP_HEAD;
public:
	CHttpResponse(void)
	{
		m_dwSetBodyLen = 0;
		m_dwLastModified = 0;
	}
	~CHttpResponse(void)
	{
	}

	inline void  begin();

	template<typename T>
	void setCookie(const string& sName, const T& tValue,const string& sDomain="",const string& sPath="",const time_t dwCookieTime=0)
	{
		std::stringstream sstr;
		sstr << tValue;
		SCookieInfo stCookieInfo;
		stCookieInfo.dwCookieTime = dwCookieTime;
		stCookieInfo.sDomain = sDomain;
		stCookieInfo.sPath = sPath;
		stCookieInfo.sValue = sstr.str();
		m_mapCookie.insert(MAP_COOKIE::value_type(sName,stCookieInfo));
	}

	inline void setCookie(const string& sName,const string& sValue,const string& sDomain="",const string& sPath="",const time_t dwCookieTime=0);
	void setStatusCode(const int iStatus) {	m_iStatusCode = iStatus;	}
	void setConnection(const string& sConn="Close")	{	m_sConnection = sConn; }
	void setContentType(const string& sData="text/html"){	m_sContentType=sData;	}
	void setCacheControl(const string& sCacheCtl) {	m_sCacheControl = sCacheCtl;	}
	void setBodyLen(const unsigned long dwLen) { m_dwSetBodyLen = dwLen;	}
	void setLocation(const string& sLocation) {	m_sLocation = sLocation;	}
	void setLastModified(const time_t dwTime) {	m_dwLastModified = dwTime;	}
	void setExpires(const time_t dwTime)	{	m_dwExpiresTime = dwTime;	}
	void setETag(const std::string& sETag)	{	m_sETag = sETag;	}
	inline void setHead(const std::string &sName,const std::string &sValue);
	inline void end();

	const char* data() const {	return m_sSendData.c_str();	}
	size_t size() const {	return static_cast<unsigned long>(m_sSendData.size());	}
	const char* getData() const {	return m_sSendData.c_str();	}
	int getDataLen() const {	return static_cast<unsigned long>(m_sSendData.size());	}

	template<typename TVal>
	this_type& operator << (const TVal& tVal){
		std::stringstream sstr;
		sstr << tVal;
		m_sBodyContent.append(sstr.str());
		return *this;
	}

	void write(const char* pData, const size_t dwSize){
		m_sBodyContent.append(pData, dwSize);
	}

	this_type& operator << (const char* pszVal){
		m_sBodyContent.append(pszVal);
		return *this;
	}

	this_type& operator << (const string& sVal){
		m_sBodyContent += sVal;
		return * this;
	}


    static const char * getStatusCodeDesc(const int iCode);

private:
	MAP_COOKIE m_mapCookie;
	MAP_HEAD m_mapHead;
	string m_sBodyContent;
	string m_sSendData;
	int m_iStatusCode;
	string m_sConnection;
	string m_sContentType;
	string m_sCacheControl;
	string m_sLocation;
	time_t m_dwLastModified;
	time_t m_dwExpiresTime;
	std::string m_sETag;
	unsigned long m_dwSetBodyLen;
};

void CHttpResponse::setHead(const std::string &sName,const std::string &sValue)
{
	m_mapHead[sName] = sValue;
}


 void CHttpResponse::setCookie(const string& sName,const string& sValue,const string& sDomain,const string& sPath,const time_t dwCookieTime)
 {

	SCookieInfo stCookieInfo;
	stCookieInfo.dwCookieTime = dwCookieTime;
	stCookieInfo.sDomain = sDomain;
	stCookieInfo.sPath = sPath;
	stCookieInfo.sValue = sValue;
	m_mapCookie.insert(MAP_COOKIE::value_type(sName,stCookieInfo));
 }



void CHttpResponse::end()
{
	char szTmp[1024]={0};
	m_sSendData.erase();

	snprintf(szTmp, sizeof(szTmp), "HTTP/1.1 %d %s\r\n", m_iStatusCode,getStatusCodeDesc(m_iStatusCode));
	m_sSendData = szTmp;

	snprintf(szTmp, sizeof(szTmp),"Connection: %s\r\n", m_sConnection.c_str());
	m_sSendData += szTmp;

	//Date: Wed, 22 Mar 2006 07:47:52 GMT\r\n
	snprintf(szTmp, sizeof(szTmp), "Date: %s\r\n", getGMTDate(time(NULL)).c_str());
	m_sSendData += szTmp;

	if (m_dwLastModified > 0)
	{
		snprintf(szTmp, sizeof(szTmp), "Last-Modified: %s\r\n", getGMTDate(m_dwLastModified).c_str());
		m_sSendData += szTmp;
	}

	if ( m_dwExpiresTime > 0 )
	{
		snprintf(szTmp, sizeof(szTmp), "Expires: %s\r\n", getGMTDate(m_dwExpiresTime).c_str());
		m_sSendData += szTmp;
	}

	if (!m_sCacheControl.empty())
	{
		snprintf(szTmp, sizeof(szTmp),"Cache-Control: %s\r\n", m_sCacheControl.c_str());
		m_sSendData += szTmp;
	}

	if ( !m_sETag.empty() )
	{
		snprintf(szTmp, sizeof(szTmp),"ETag: %s\r\n", m_sETag.c_str());
		m_sSendData += szTmp;
	}

	//length
	if (m_dwSetBodyLen != 0)
	{
		snprintf(szTmp, sizeof(szTmp), "Content-Length: %lu\r\n", m_dwSetBodyLen);
		m_sSendData += szTmp;
	}
	else
	{
		if ( !m_sBodyContent.empty() )
		{
			snprintf(szTmp, sizeof(szTmp), "Content-Length: %d\r\n", (uint32_t)m_sBodyContent.size());
			m_sSendData += szTmp;
		}
	}

	for(MAP_HEAD::const_iterator it = m_mapHead.begin();it!=m_mapHead.end();++it)
	{
		snprintf(szTmp, sizeof(szTmp), "%s: %s\r\n", it->first.c_str(),it->second.c_str());
		m_sSendData += szTmp;
	}

	//set cookie
	for (MAP_COOKIE::const_iterator it=m_mapCookie.begin(); it!=m_mapCookie.end(); ++it)
	{
		m_sSendData += "Set-Cookie:"  + it->first + "=" + it->second.sValue;
			if ( !it->second.sDomain.empty() )
				m_sSendData += "; Domain=" + it->second.sDomain;
			if ( it->second.dwCookieTime != 0 )
				m_sSendData += "; Expires=" + getGMTDate(it->second.dwCookieTime+time(NULL));
			if ( !it->second.sPath.empty() )
				m_sSendData += "; Path=" + it->second.sPath;
			m_sSendData += "\r\n";
	}

	if ( !m_sLocation.empty() )
	{
		m_sSendData += "Location:" + m_sLocation + "\r\n";
	}

	//html head end
	snprintf(szTmp, sizeof(szTmp), "Content-Type: %s\r\n\r\n", m_sContentType.c_str());
	m_sSendData += szTmp;
	m_sSendData += m_sBodyContent;
}

void CHttpResponse::begin()
{
	m_mapCookie.clear();
	m_sBodyContent.erase();
	m_iStatusCode = 200;
	m_sConnection = "Close";
	m_sContentType = "text/html";
	m_dwSetBodyLen = 0;
	m_sCacheControl.erase();
	m_sLocation.erase();
	m_dwLastModified = 0;
	m_sETag.clear();
	m_dwExpiresTime = 0;
}

};

#endif


