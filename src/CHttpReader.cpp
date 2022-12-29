#include "CHttpReader.h"
#include <iostream>
using namespace std;

namespace lce
{

std::string CHttpReader::m_sNull = "";

CHttpReader::CHttpReader(void)
{
	memset(m_szNULL, 0, sizeof(m_szNULL));
	memset(m_szErrMsg, 0, sizeof(m_szErrMsg));
}

CHttpReader::~CHttpReader(void)
{
}

bool CHttpReader::setData(const char *pszData, const int iDataLen)
{
	bool bOk = true;
	m_sHttpContent.assign(pszData, iDataLen);
	this->reset();

	string sValue;
	string sName;
	string::size_type pos1 = 0;
	string::size_type pos2 = 0;

	string::size_type headEndPos = m_sHttpContent.find("\r\n\r\n");

	//http command
	pos2 = m_sHttpContent.find(' ', pos1);
	if (pos2 != string::npos)
	{
		sValue = m_sHttpContent.substr(0,pos2);
		if (sValue == "GET")
		{
			m_nCommand = GET;
		}
		else if (sValue == "POST")
		{
			m_nCommand = POST;
		}
		else if (sValue == "HEAD")
		{
			m_nCommand = HEAD;
		}
		else
		{
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "http command error: %s.", sValue.c_str());
			return false;
		}
	}
	else
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "parse http command error: can't find space.");
		return false;
	}

	//uri
	pos1 = pos2 + 1;
	pos2 = m_sHttpContent.find(' ', pos1);
	if (string::npos != pos2)
	{
		m_sURI = m_sHttpContent.substr(pos1, pos2-pos1);
		if ( m_sURI.empty() )
		{
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "URI error: uri is empty.");
			return false;
		}


		if (m_nCommand == GET)
		{
			string::size_type tmpPos = m_sURI.find('?');
			if (tmpPos != string::npos)
			{
				m_sValues = m_sURI.substr(tmpPos+1);
				m_sURI.erase(tmpPos);
			}
		}
		else if ( m_nCommand == POST )
		{
			string::size_type tmpPos = m_sURI.find('?');
			if (tmpPos != string::npos)
			{
				m_sURI.erase(tmpPos);
			}

			m_sValues = m_sHttpContent.substr(headEndPos+4);
		}
	}
	else
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "parse http URI error: can't find space.");
		return false;
	}

	//version
	pos1 = pos2 + 1;
	pos2 = m_sHttpContent.find("\r\n", pos1);
	if (pos2 != string::npos)
	{
		m_sVersion = m_sHttpContent.substr( pos1, pos2 - pos1 );
		if ( m_sVersion.empty() )
		{
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "version error: version is empty.");
			return false;
		}
	}
	else
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "parse http version error: can't find <\\r\\n>.");
		return false;
	}

	pos1 = pos2 + 2;
	string sLine;
	string::size_type colonPos = 0;

	string sBoundary;
	while ( string::npos != pos1 && pos1 < headEndPos )
	{
		pos2 = m_sHttpContent.find("\r\n", pos1);
		if ( pos2 == string::npos || headEndPos < pos2 )
			break;

		sLine = m_sHttpContent.substr(pos1, pos2-pos1);
		pos1 = pos2 + 2;

		colonPos = sLine.find(": ");
		sName = sLine.substr( 0, colonPos );
		if (sName.empty())
		{
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "name error: name is empty<pos1=%d, pos2=%d>.", (uint32_t)pos1, (uint32_t)pos2);
			return false;
		}

		sValue = sLine.substr( colonPos+2 );

		static const string BOUNDARY_STR("multipart/form-data; boundary=");
		if(sBoundary.empty() && sName == "Content-Type" && sValue.substr(0, BOUNDARY_STR.length()) == BOUNDARY_STR)
		{
			sBoundary = sValue.substr(BOUNDARY_STR.length());
		}
		else
		{
			m_mapHeadInfo[toLower(sName.c_str())] = sValue;
		}
	}

	this->parseCookies();
	this->parseValues();


	if(m_nCommand == POST && !sBoundary.empty())
	{
		sBoundary.insert(0, "--");
		this->parsePostFormData(sBoundary, headEndPos);
	}

	return bOk;
}

void CHttpReader::reset()
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

string& CHttpReader::getCookie(const string& sName, string& sValue, const string& sDefVal) const
{
	sValue = sDefVal;
	MAP_COOKIE::const_iterator it = m_mapCookieList.find(sName);
	if ( it != m_mapCookieList.end() )
	{
		sValue = it->second;
	}
	return sValue;
}

unsigned long CHttpReader::getCookie(const string& sName, unsigned long& dwValue, const unsigned long dwDefVal) const
{
	dwValue = dwDefVal;
	MAP_COOKIE::const_iterator it = m_mapCookieList.find(sName);
	if ( it != m_mapCookieList.end() )
	{
		dwValue = atoll(it->second.c_str());
	}
	return dwValue;
}

int CHttpReader::getCookie(const string& sName, int& iValue, const int iDefVal) const
{
	iValue = iDefVal;
	MAP_COOKIE::const_iterator it = m_mapCookieList.find(sName);
	if ( it != m_mapCookieList.end() )
	{
		iValue = atoi(it->second.c_str());
	}
	return iValue;
}

unsigned char CHttpReader::getCookie(const string& sName, unsigned char& ucValue, const unsigned char ucDefVal) const
{
	ucValue = ucDefVal;
	MAP_COOKIE::const_iterator it = m_mapCookieList.find(sName);
	if ( it != m_mapCookieList.end() )
	{
		ucValue = static_cast<unsigned char>(atoi(it->second.c_str()));
	}
	return ucValue;
}

unsigned short CHttpReader::getCookie(const string sName, int& wValue, const unsigned short wDefVal) const
{
	wValue = wDefVal;
	MAP_COOKIE::const_iterator it = m_mapCookieList.find(sName);
	if ( it != m_mapCookieList.end() )
	{
		wValue = atol(it->second.c_str());
	}
	return wValue;
}


unsigned char CHttpReader::getValue(const string& sName, unsigned char& ucValue, const unsigned char ucDefVal) const
{
	ucValue = ucDefVal;
	MAP_COOKIE::const_iterator it = m_mapValueList.find(sName);
	if ( it != m_mapValueList.end() )
	{
		ucValue = static_cast<unsigned char>(atol(it->second.c_str()));
	}
	return ucValue;
}

string& CHttpReader::getValue(const string& sName, string& sValue, const string& sDefVal) const
{
	sValue = sDefVal;

	MAP_VALUE::const_iterator it = m_mapValueList.find(sName);
	if ( it != m_mapValueList.end() )
	{
		sValue = it->second;
	}
	return sValue;
}


string& CHttpReader::getValue(string& sValue) const
{
	sValue = m_sValues;
	return sValue;
}

unsigned long CHttpReader::getValue(const string& sName,  unsigned long& dwValue, const unsigned long dwDefVal) const
{
	dwValue = dwDefVal;

	MAP_VALUE::const_iterator it = m_mapValueList.find(sName);
	if ( it != m_mapValueList.end() )
	{
		dwValue = atoll(it->second.c_str());
	}
	return dwValue;
}

bool CHttpReader::getValue(const string& sName, bool& bValue, const bool bDefVal) const
{
	bValue = bDefVal;
	MAP_VALUE::const_iterator it = m_mapValueList.find(sName);
	if ( it != m_mapValueList.end() )
	{
		bValue = atol(it->second.c_str())==0 ? false : true;
	}
	return bValue;

}

int CHttpReader::getValue(const string& sName, int& iValue, const int iDefVal) const
{
	iValue = iDefVal;

	MAP_VALUE::const_iterator it = m_mapValueList.find(sName);
	if ( it != m_mapValueList.end() )
	{
		iValue = atoi(it->second.c_str());
	}
	return iValue;
}

unsigned short CHttpReader::getValue(const string& sName, unsigned short& wValue, const unsigned short wDefVal) const
{
	wValue = wDefVal;

	MAP_VALUE::const_iterator it = m_mapValueList.find(sName);
	if ( it != m_mapValueList.end() )
	{
		wValue = static_cast<unsigned short>(atol(it->second.c_str()));
	}
	return wValue;
}


void CHttpReader::parseCookies()
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

		std::string::size_type wscount = 0;
		std::string::const_iterator data_iter;
		for(data_iter = sTmp.begin(); data_iter != sTmp.end(); ++data_iter,++wscount)
			if(isspace(*data_iter) == 0)
				break;

		m_mapCookieList[sTmp.substr(wscount, equalPos - wscount)] = FormUrlDecode(sTmp.substr(1+equalPos));
	}
}

void CHttpReader::parseValues()
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
		m_mapValueList[sTmp.substr(0, equalPos)] = FormUrlDecode(sTmp.substr(1+equalPos));
	}
}



const char* CHttpReader::getHead(const string& sName) const
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

string CHttpReader::getStrBetween(const string &sOrig, const string::size_type iPos, const string &sHead, const string &sTail, string::size_type &iFoundPos)
{
	iFoundPos = string::npos;

	string::size_type idx1, idx2;

	idx1 = sOrig.find(sHead, iPos);
	if(idx1 == string::npos)
	{
		return "";
	}

	idx2 = sOrig.find(sTail, idx1+sHead.size());
	if(idx2 == string::npos)
	{
		return "";
	}

	iFoundPos = idx1+sHead.size();

	return sOrig.substr(idx1+sHead.size(), idx2 - idx1 - sHead.size());
}

void CHttpReader::parsePostFormData(const string &sBoundary, const string::size_type headEndPos)
{
	string sValue;
	string sName;
	string::size_type pos1 = headEndPos+4;

	while ( string::npos != pos1 && pos1 < m_sHttpContent.size())
	{
		string::size_type pos = 0;
		string sNV = getStrBetween(m_sHttpContent, pos1, sBoundary, sBoundary, pos);

		if(pos == string::npos || sNV.empty())
		{
			break;
		}

		pos1 = pos + sNV.size() ;

		static const string FORM_DATA("Content-Disposition: form-data; ");
		if(sNV.substr(2, FORM_DATA.size()) != FORM_DATA)
		{
			break;
		}

		sName = getStrBetween(sNV, 2, "name=\"", "\"", pos);
		if(pos == string::npos || sName.empty())
		{
			break;
		}

		pos = sNV.find("\r\n\r\n", pos+1);
		if(pos == string::npos)
		{
			break;
		}
		sValue = sNV.substr(pos+4);
		if(sValue.size() > 2)
		{
			sValue.erase(sValue.size() -2);
		}
		m_mapValueList[sName] = FormUrlDecode(sValue);
		m_sFileName = getStrBetween(sNV, 0, "filename=\"", "\"", pos);
		if(pos == string::npos || m_sFileName.empty())
		{
			continue;
		}

		string sFileType = getStrBetween(sNV, pos, "Content-Type: ", "\r\n", pos);
		if(pos == string::npos || sFileType.empty())
		{
			continue;
		}
		SFile stFile;
		m_mapFiles[sName] = stFile;

		MAP_FILE::iterator it = m_mapFiles.find(sName);

		it->second.sData = sNV.substr(pos+sFileType.size()+4);

		if(it->second.sData.size() > 2)
		{
			it->second.sData.erase(it->second.sData.size()-2);
		}

		it->second.sContentType =sFileType;
		it->second.sName = m_sFileName;
	}

}

};
