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

class CHttpWriter
{
	typedef CHttpWriter this_type;
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
	CHttpWriter(void)
	{
		m_dwSetBodyLen = 0;
		m_dwLastModified = 0;
	}
	~CHttpWriter(void)
	{
	}

	void  begin();

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

	void setCookie(const string& sName,const string& sValue,const string& sDomain="",const string& sPath="",const time_t dwCookieTime=0);
	void setStatusCode(const int iStatus) {	m_iStatusCode = iStatus;	}
	void setConnection(const string& sConn="Close")	{	m_sConnection = sConn; }
	void setContentType(const string& sData="text/html"){	m_sContentType=sData;	}
	void setCacheControl(const string& sCacheCtl) {	m_sCacheControl = sCacheCtl;	}
	void setBodyLen(const unsigned long dwLen) { m_dwSetBodyLen = dwLen;	}
	void setLocation(const string& sLocation) {	m_sLocation = sLocation;	}
	void setLastModified(const time_t dwTime) {	m_dwLastModified = dwTime;	}
	void setExpires(const time_t dwTime)	{	m_dwExpiresTime = dwTime;	}
	void setETag(const std::string& sETag)	{	m_sETag = sETag;	}
	void setHead(const std::string &sName,const std::string &sValue);
	void end();

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

};

#endif


