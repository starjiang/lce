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

class CHttpReader
{
public:
	CHttpReader(void);
	~CHttpReader(void);

	bool setData(const char* pszData, const int iDataLen);
//	inline bool SetData(const unsigned char* pszData, const int iDataLen);

	const char* getBody() const{ return m_sValues.c_str();}
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
	time_t getIfModifiedSinceTime() const   {	return Gmt2Time(getHead("If-Modified-Since"));	}
	time_t getIfUnmodifiedSinceTime() const   {	return Gmt2Time(getHead("If-Unmodified-Since"));	}
	const char* getIfMatch() const   {	return getHead("If-Match");	}
	const char* getIfNoneMatch() const   {	return getHead("If-None-Match");	}
	const char* getIfRange() const   {	return getHead("If-Range");	}


	const char* getHead(const string& sName)  const ;

	const char* getCookieList() const {	return m_sCookies.c_str();	}

	string& getCookie(const string& sName, string& sValue, const string& sDefVal="") const ;
	unsigned long getCookie(const string& sName, unsigned long& dwValue, const unsigned long dwDefVal=0) const ;
	int getCookie(const string& sName, int& iValue, const int iDefVal=0) const ;
	unsigned short getCookie(const string sName, int& wValue, const unsigned short wDefVal=0) const ;
	unsigned char getCookie(const string& sName, unsigned char& ucValue, const unsigned char ucDefVal=0) const;

	const map<string, string>& getValueList() const {	return m_mapValueList;	}
	string& getValue(const string& sName, string& sValue, const string& sDefVal="") const ;
	string& getValue(string& sValue) const ;
	unsigned long getValue(const string& sName, unsigned long& dwValue, const unsigned long dwDefVal=0) const ;
	int getValue(const string& sName, int& iValue, const int iDefVal=0) const ;
	unsigned short getValue(const string& sName, unsigned short& wValue, const unsigned short wDefVal=0) const ;
	bool getValue(const string& sName, bool& bValue, const bool bDefVal=false) const;
	unsigned char getValue(const string& sName, unsigned char& ucValue, const unsigned char ucDefVal=0) const;

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

	CHttpReader(const CHttpReader& rhs);
	CHttpReader& operator=(const CHttpReader& rhs);

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

};

#endif
