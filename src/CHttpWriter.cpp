#include "CHttpWriter.h"

namespace lce
{

void CHttpWriter::setHead(const std::string &sName,const std::string &sValue)
{
	m_mapHead[sName] = sValue;
}

void CHttpWriter::setCookie(const string& sName,const string& sValue,const string& sDomain,const string& sPath,const time_t dwCookieTime)
{

	StCookieInfo stCookieInfo;
	stCookieInfo.dwCookieTime = dwCookieTime;
	stCookieInfo.sDomain = sDomain;
	stCookieInfo.sPath = sPath;
	stCookieInfo.sValue = sValue;
	m_mapCookie.insert(MAP_COOKIE::value_type(sName,stCookieInfo));
}

void CHttpWriter::end()
{
	char szTmp[1024]={0};
	m_sSendData.erase();

	snprintf(szTmp, sizeof(szTmp), "HTTP/1.1 %d %s\r\n", m_iStatusCode,getStatusCodeDesc(m_iStatusCode));
	m_sSendData = szTmp;

	snprintf(szTmp, sizeof(szTmp),"Connection: %s\r\n", m_sConnection.c_str());
	m_sSendData += szTmp;

	//Date: Wed, 22 Mar 2006 07:47:52 GMT\r\n
	snprintf(szTmp, sizeof(szTmp), "Date: %s\r\n", GetGMTDate(time(NULL)).c_str());
	m_sSendData += szTmp;

	if (m_dwLastModified > 0)
	{
		snprintf(szTmp, sizeof(szTmp), "Last-Modified: %s\r\n", GetGMTDate(m_dwLastModified).c_str());
		m_sSendData += szTmp;
	}

	if ( m_dwExpiresTime > 0 )
	{
		snprintf(szTmp, sizeof(szTmp), "Expires: %s\r\n", GetGMTDate(m_dwExpiresTime).c_str());
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
				m_sSendData += "; Expires=" + GetGMTDate(it->second.dwCookieTime+time(NULL));
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

void CHttpWriter::begin()
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

const char * CHttpWriter::getStatusCodeDesc(const int iCode)
{
    switch(iCode)
    {
        case 100 : return "Continue";
        case 101 : return "Witching Protocols";
        case 200 : return "OK";
        case 201 : return "Created";
        case 202 : return "Accepted";
        case 203 : return "Non-Authoritative Information";
        case 204 : return "No Content";
        case 205 : return "Reset Content";
        case 206 : return "Partial Content";
        case 300 : return "Multiple Choices";
        case 301 : return "Moved Permanently";
        case 302 : return "Found";
        case 303 : return "See Other";
        case 304 : return "Not Modified";
        case 305 : return "Use Proxy";
        case 307 : return "Temporary Redirect";
        case 400 : return "Bad Request";
        case 401 : return "Unauthorized";
        case 402 : return "Payment Required";
        case 403 : return "Forbidden";
        case 404 : return "Not Found";
        case 405 : return "Method Not Allowed";
        case 406 : return "Not Acceptable";
        case 407 : return "Proxy Authentication Required";
        case 408 : return "Request Time-out";
        case 409 : return "Conflict";
        case 410 : return "Gone";
        case 411 : return "Length Required";
        case 412 : return "Precondition Failed";
        case 413 : return "Request Entity Too Large";
        case 414 : return "Request-URI Too Large";
        case 415 : return "Unsupported Media Type";
        case 416 : return "Requested range not satisfiable";
        case 417 : return "Expectation Failed";
        case 500 : return "Internal Server Error";
        case 501 : return "Not Implemented";
        case 502 : return "Bad Gateway";
        case 503 : return "Service Unavailable";
        case 504 : return "Gateway Time-out";
        case 505 : return "HTTP Version not supported";
        default : return "OK";
    }
}

};

