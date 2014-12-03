#ifndef __HTTPPARSER_H__
#define __HTTPPARSER_H__

#include <map>
#include <string>
#include "Utils.h"
using std::map;
using std::string;

namespace lce
{

enum HTTP_COMMAND{
	GET,
	POST,
	HEAD
};

struct SFile 
{
	std::string sData;
	std::string sName;
	std::string sContentType;

};
struct SMIMEHeader{
	std::string sDisposition;
	std::string sName;
	std::string sFileName;
	std::string sContentType;
};

typedef std::map<std::string, std::string> MAP_COOKIE;
typedef std::map<std::string, std::string> MAP_VALUE;
typedef std::map<std::string, SFile> MAP_FILE;
typedef map<string, string > MAP_HEAD_INFO;

class CHttpParser
{
public:
	CHttpParser(void);
	~CHttpParser(void);

	bool setData(const char* pszData, const int iDataLen);
//	inline bool SetData(const unsigned char* pszData, const int iDataLen);

	HTTP_COMMAND getCommand() const {	return m_nCommand;	}
	const char* getURI() const {	return m_sURI.c_str();	}
	const char* getVersion() const {	return m_sVersion.c_str();	}
	const char* getAccept() const   {	return getHead("Accept"); }
	const char* getReferer() const  {	return getHead("Referer");	}
	const char* getAcceptLanguage() const   {	return getHead("Accept-Language");  }
	const char* getContentType() const   {	return getHead("Content-Type");	  }
	const char* getAcceptEncoding() const   {	return getHead("Accept-Encoding");	  }
	const char* getUserAgent() const   {	return getHead("User-Agent");	  }
	const char* getHost() const   {	return getHead("Host");		}
	unsigned long getContentLength() const   {	return atol(getHead("Content-Length"));	}
	const char* getConnection() const   {	return getHead("Connection");	}
	const char* getCacheControl() const   {	return getHead("Cache-Control");	}
	time_t getIfModifiedSinceTime() const   {	return gmt2Time(getHead("If-Modified-Since"));	}
	time_t getIfUnmodifiedSinceTime() const   {	return gmt2Time(getHead("If-Unmodified-Since"));	}
	const char* getIfMatch() const   {	return getHead("If-Match");	}
	const char* getIfNoneMatch() const   {	return getHead("If-None-Match");	}
	const char* getIfRange() const   {	return getHead("If-Range");	}


	inline const char* getHead(const string& sName)  const ;

	const char* getCookieList() const {	return m_sCookies.c_str();	}

	inline string& getCookie(const string& sName, string& sValue, const string& sDefVal="") const ;
	inline unsigned long getCookie(const string& sName, unsigned long& dwValue, const unsigned long dwDefVal=0) const ;
	inline int getCookie(const string& sName, int& iValue, const int iDefVal=0) const ;
	inline unsigned short getCookie(const string sName, int& wValue, const unsigned short wDefVal=0) const ;
	inline unsigned char getCookie(const string& sName, unsigned char& ucValue, const unsigned char ucDefVal=0) const;

	const map<string, string>& getValueList() const {	return m_mapValueList;	}
	inline string& getValue(const string& sName, string& sValue, const string& sDefVal="") const ;
	inline string& getValue(string& sValue) const ;
	inline unsigned long getValue(const string& sName, unsigned long& dwValue, const unsigned long dwDefVal=0) const ;
	inline int getValue(const string& sName, int& iValue, const int iDefVal=0) const ;
	inline unsigned short getValue(const string& sName, unsigned short& wValue, const unsigned short wDefVal=0) const ;
	inline bool getValue(const string& sName, bool& bValue, const bool bDefVal=false) const;
	inline unsigned char getValue(const string& sName, unsigned char& ucValue, const unsigned char ucDefVal=0) const;

	const char* getErrMsg() const {	return m_szErrMsg;	}

	const string& getFile(const std::string& sName) const  {

		MAP_FILE::const_iterator it = m_mapFiles.find(sName);
		if ( it != m_mapFiles.end() )
		{
			return it->second.sData;
		}
		return m_sNull;		
	}
	const string& getFileName(const std::string& sName) const  {			
		MAP_FILE::const_iterator it = m_mapFiles.find(sName);
		if ( it != m_mapFiles.end() )
		{
			return it->second.sName;
		}
		return m_sNull;	 
	}

	const string& getFileType(const std::string& sName) const  {			
		MAP_FILE::const_iterator it = m_mapFiles.find(sName);
		if ( it != m_mapFiles.end() )
		{
			return it->second.sContentType;
		}
		return m_sNull;	 
	}

	void clear(){
		m_sURI.clear();
		m_sVersion.clear();
		m_mapCookieList.clear();
		m_mapValueList.clear();
		m_sCookies.erase();
		m_sValues.erase();
		m_mapHeadInfo.clear();
		m_sHttpContent.clear();
		m_sFile.clear();
		m_sFileName.clear();
	}
private:
	inline void reset();
	inline void parseValues();
	inline void parseCookies();
	void parsePostFormData(const string &sBoundary, const string::size_type headEndPos);

	string getStrBetween(const string &sOrig, const string::size_type iPos, const string &sHead, const string &sTail, string::size_type &iFoundPos);

	CHttpParser(const CHttpParser& rhs);
	CHttpParser& operator=(const CHttpParser& rhs);

	inline string toLower(const char* pData) const
	{
		string sData;
		int iLen = strlen(pData);
		for (int i=0; i<iLen; ++i)
			sData += static_cast<char>(tolower(*(pData+i)));

		return sData;
	}
private:
	char m_szNULL[2];
	char m_szErrMsg[1024];
	HTTP_COMMAND m_nCommand;
	string m_sURI;
	string m_sVersion;
	string m_sCookies;
	string m_sValues;
	MAP_COOKIE m_mapCookieList;
	MAP_VALUE	m_mapValueList;

	MAP_HEAD_INFO m_mapHeadInfo;

	string m_sHttpContent;

	string m_sFile;
	string m_sFileName;

	MAP_FILE m_mapFiles;

	static std::string m_sNull;

};

void CHttpParser::reset()
{
	m_sURI.erase();
	m_sVersion.erase();
	m_mapCookieList.clear();
	m_mapValueList.clear();
	m_sCookies.erase();
	m_sValues.erase();
	m_mapHeadInfo.clear();
	m_sFile.clear();
	m_sFileName.clear();

}

string& CHttpParser::getCookie(const string& sName, string& sValue, const string& sDefVal) const
{
	sValue = sDefVal;
	MAP_COOKIE::const_iterator it = m_mapCookieList.find(sName);
	if ( it != m_mapCookieList.end() )
	{
		sValue = it->second;
	}
	return sValue;
}

unsigned long CHttpParser::getCookie(const string& sName, unsigned long& dwValue, const unsigned long dwDefVal) const
{
	dwValue = dwDefVal;
	MAP_COOKIE::const_iterator it = m_mapCookieList.find(sName);
	if ( it != m_mapCookieList.end() )
	{
		dwValue = atoll(it->second.c_str());
	}
	return dwValue;
}

int CHttpParser::getCookie(const string& sName, int& iValue, const int iDefVal) const
{
	iValue = iDefVal;
	MAP_COOKIE::const_iterator it = m_mapCookieList.find(sName);
	if ( it != m_mapCookieList.end() )
	{
		iValue = atoi(it->second.c_str());
	}
	return iValue;
}

unsigned char CHttpParser::getCookie(const string& sName, unsigned char& ucValue, const unsigned char ucDefVal) const
{
	ucValue = ucDefVal;
	MAP_COOKIE::const_iterator it = m_mapCookieList.find(sName);
	if ( it != m_mapCookieList.end() )
	{
		ucValue = static_cast<unsigned char>(atoi(it->second.c_str()));
	}
	return ucValue;
}

unsigned short CHttpParser::getCookie(const string sName, int& wValue, const unsigned short wDefVal) const
{
	wValue = wDefVal;
	MAP_COOKIE::const_iterator it = m_mapCookieList.find(sName);
	if ( it != m_mapCookieList.end() )
	{
		wValue = atol(it->second.c_str());
	}
	return wValue;
}


unsigned char CHttpParser::getValue(const string& sName, unsigned char& ucValue, const unsigned char ucDefVal) const
{
	ucValue = ucDefVal;
	MAP_COOKIE::const_iterator it = m_mapValueList.find(sName);
	if ( it != m_mapValueList.end() )
	{
		ucValue = static_cast<unsigned char>(atol(it->second.c_str()));
	}
	return ucValue;
}

string& CHttpParser::getValue(const string& sName, string& sValue, const string& sDefVal) const
{
	sValue = sDefVal;

	MAP_VALUE::const_iterator it = m_mapValueList.find(sName);
	if ( it != m_mapValueList.end() )
	{
		sValue = it->second;
	}
	return sValue;
}


string& CHttpParser::getValue(string& sValue) const
{
	sValue = m_sValues;
	return sValue;
}

unsigned long CHttpParser::getValue(const string& sName,  unsigned long& dwValue, const unsigned long dwDefVal) const
{
	dwValue = dwDefVal;

	MAP_VALUE::const_iterator it = m_mapValueList.find(sName);
	if ( it != m_mapValueList.end() )
	{
		dwValue = atoll(it->second.c_str());
	}
	return dwValue;
}

bool CHttpParser::getValue(const string& sName, bool& bValue, const bool bDefVal) const
{
	bValue = bDefVal;
	MAP_VALUE::const_iterator it = m_mapValueList.find(sName);
	if ( it != m_mapValueList.end() )
	{
		bValue = atol(it->second.c_str())==0 ? false : true;
	}
	return bValue;

}

int CHttpParser::getValue(const string& sName, int& iValue, const int iDefVal) const
{
	iValue = iDefVal;

	MAP_VALUE::const_iterator it = m_mapValueList.find(sName);
	if ( it != m_mapValueList.end() )
	{
		iValue = atoi(it->second.c_str());
	}
	return iValue;
}

unsigned short CHttpParser::getValue(const string& sName, unsigned short& wValue, const unsigned short wDefVal) const
{
	wValue = wDefVal;

	MAP_VALUE::const_iterator it = m_mapValueList.find(sName);
	if ( it != m_mapValueList.end() )
	{
		wValue = static_cast<unsigned short>(atol(it->second.c_str()));
	}
	return wValue;
}


void CHttpParser::parseCookies()
{
	string::size_type pos1 = 0;
	string::size_type pos2 = 0;
	string::size_type equalPos = 0;
	string sTmp;

	MAP_HEAD_INFO::const_iterator it = m_mapHeadInfo.find("cookie");
	if (it != m_mapHeadInfo.end())
	{
		m_sCookies = it->second;
	}

	if (m_sCookies.empty())
		return ;
	while ( pos2 != string::npos )
	{
		pos2 = m_sCookies.find(';',pos1);

		sTmp = m_sCookies.substr(pos1, pos2-pos1);
		pos1 = pos2+1;
		equalPos = sTmp.find('=');
		if (equalPos == string::npos)
			continue;

		// skip leading whitespace - " \f\n\r\t\v"
		std::string::size_type wscount = 0;
		std::string::const_iterator data_iter;
		for(data_iter = sTmp.begin(); data_iter != sTmp.end(); ++data_iter,++wscount)
			if(isspace(*data_iter) == 0)
				break;

		m_mapCookieList[sTmp.substr(wscount, equalPos - wscount)] = formUrlDecode(sTmp.substr(1+equalPos));
	}
}

void CHttpParser::parseValues()
{
	string::size_type pos1 = 0;
	string::size_type pos2 = 0;
	string::size_type equalPos = 0;
	string sTmp;

	if (m_sValues.empty())
		return ;
	while ( pos2 != string::npos )
	{
		pos2 = m_sValues.find('&',pos1);
		sTmp = m_sValues.substr(pos1, pos2-pos1);
		pos1 = pos2+1;
		equalPos = sTmp.find('=');
		if (equalPos == string::npos)
			continue;
		m_mapValueList[sTmp.substr(0, equalPos)] = formUrlDecode(sTmp.substr(1+equalPos));
	}
}



const char* CHttpParser::getHead(const string& sName) const
{
	MAP_HEAD_INFO::const_iterator it = m_mapHeadInfo.find(toLower(sName.c_str()));
	if ( it != m_mapHeadInfo.end() )
	{
		return it->second.c_str();
	}
	else
	{
		return m_szNULL;
	}

}

};

#endif
