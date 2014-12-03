#include "CHttpParser.h"
#include <iostream>
using namespace std;

namespace lce
{

std::string CHttpParser::m_sNull = "";

CHttpParser::CHttpParser(void)
{
	memset(m_szNULL, 0, sizeof(m_szNULL));
	memset(m_szErrMsg, 0, sizeof(m_szErrMsg));
}

CHttpParser::~CHttpParser(void)
{
}

bool CHttpParser::setData(const char *pszData, const int iDataLen)
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

	string sBoundary;//2010-02-26 解析post 的formdata等
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

	//2010-02-26 解析post 的formdata等
	if(m_nCommand == POST && !sBoundary.empty())
	{
		sBoundary.insert(0, "--");
		this->parsePostFormData(sBoundary, headEndPos);
	}

	return bOk;
}

string CHttpParser::getStrBetween(const string &sOrig, const string::size_type iPos, const string &sHead, const string &sTail, string::size_type &iFoundPos)
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

	//cout << "+++++idx1=" << idx1 << ", idx2=" << idx2 << ", found=" << iFoundPos << endl;
	return sOrig.substr(idx1+sHead.size(), idx2 - idx1 - sHead.size());
}

void CHttpParser::parsePostFormData(const string &sBoundary, const string::size_type headEndPos)
{
	string sValue;
	string sName;
	string::size_type pos1 = headEndPos+4;

	//cout << "++++++++sBoundary=" << sBoundary << ",,,size=" << sBoundary.size()<< endl;

	while ( string::npos != pos1 && pos1 < m_sHttpContent.size())
	{
		string::size_type pos = 0;
		string sNV = getStrBetween(m_sHttpContent, pos1, sBoundary, sBoundary, pos);

		//cout << "sNV=" << sNV << endl << ",  pos="  << pos << endl;

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
		if(sValue.size() > 2)	//去掉最后的\r\n
		{
			sValue.erase(sValue.size() -2);
		}

		//cout << "++++form-data: " << sName << "=" << sValue << endl;

		m_mapValueList[sName] = formUrlDecode(sValue);

		//取上传文件的内容
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

		if(it->second.sData.size() > 2)	//去掉最后的\r\n
		{
			it->second.sData.erase(it->second.sData.size()-2);
		}

		it->second.sContentType =sFileType;
		it->second.sName = m_sFileName;
	
		//cout << "+++file size=" << m_sFile.size() << ",content="<< m_sFile << endl;
	}

}

};
