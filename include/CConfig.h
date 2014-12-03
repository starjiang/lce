#ifndef __NCE_CONFIG_H__
#define __NCE_CONFIG_H__

#include <string>
#include <map>
#include <vector>
#include "StringHelper.h"
#include "Utils.h"
namespace lce
{

class CConfig
{
	typedef std::map<std::string, std::string> CONFIG_VALUE;
	typedef std::map<std::string, CONFIG_VALUE > MAPCONFIG;
public:
	CConfig(void);
	~CConfig(void);

	bool loadConfig(const std::string& sCfgFileName);

	std::string& getValue(const std::string& sApp, const std::string& sName, std::string& sValue, const std::string& sDefault = "");
	int getValue(const std::string& sApp, const std::string& sName,int& iValue, const int iDefault = 0);
	unsigned int getValue(const std::string& sApp, const std::string& sName,unsigned int& uiValue, const unsigned int uiDefault = 0);
	long getValue(const std::string& sApp, const std::string& sName,long& lValue, const long lDefault = 0);
	unsigned short getValue(const std::string& sApp, const std::string& sName,unsigned short& wValue, const unsigned short wDefault=0);
    unsigned char getValue(const std::string& sApp, const std::string& sName,unsigned char& cValue, const unsigned char cDefault=0);
	unsigned long getValue(const std::string& sApp, const std::string& sName,unsigned long& dwValue, const unsigned long dwDefault=0);
	bool getValue(const std::string& sApp, const std::string& sName,bool& bValue, const bool bDefault = false );

	bool getIPAndPort(const std::string& sApp, const std::string& sName, std::string &sIP, unsigned short& wPort, const std::string &sDefaultIP, const unsigned short wDefaultPort);
	bool getIPAndPortList(const std::string& sApp, const std::string& sName, std::vector<std::pair<std::string, unsigned short> >& vecSvrs, const std::string &sDefaultIP, const unsigned short wDefaultPort);

	const char* getErrMsg() const {	return m_szErrMsg;	}
private:
	CConfig(const CConfig& rhs);
	CConfig& operator=(const CConfig& rhs);
private:
	std::string m_sCfgFileName;
	char m_szErrMsg[1024];
	MAPCONFIG m_mapConfig;
};

};
#endif

