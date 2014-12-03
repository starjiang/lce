#include "CConfig.h"
#include <errno.h>
#include <fstream>


namespace lce
{

CConfig::CConfig(void)
{
}

CConfig::~CConfig(void)
{
}

bool CConfig::loadConfig(const std::string& sCfgFileName)
{
	std::ifstream ifsConfig;
	ifsConfig.open(sCfgFileName.c_str());
	if (ifsConfig.fail())
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"errno=%d",errno);
		return false;
	}

	std::string sApp;
	std::string sName;
	std::string sValue;
	std::string sLine;
	while (getline(ifsConfig,sLine))
	{
		if (sLine.empty())
		{
			continue;
		}

		size_t i = 0;
		for(i = 0; i < sLine.size(); i++)
		{
			if(sLine[i] != ' ' || sLine[i] != '\t')
			{
				break;
			}
		}

		switch(sLine[i]) {
		case '#':
		case ';':
			break;
		case '[':
			{
				size_t j = sLine.find(']', i);
				if(std::string::npos != j)
				{
					sApp = sLine.substr(i+1, j-i-1);
					lce::trimStr(sApp);
					if (sApp.empty())
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
		default:
			size_t j = sLine.find('=', i);
			if(j > i)
			{
				sName = sLine.substr(i, j-i);

				lce::trimStr(sName);
				lce::trimStr(sApp);

				if(!sName.empty())
				{
					sValue = sLine.substr(j+1);
					trimStr(sValue);
					m_mapConfig[sApp][sName] = sValue;
				}
			}
			break;
		}
	}

	return true;
}

std::string& CConfig::getValue(const std::string& sApp, const std::string& sName, std::string& sValue, const std::string& sDefault)
{
	sValue = sDefault;
	MAPCONFIG::iterator it = m_mapConfig.find(sApp);
	if (it != m_mapConfig.end())
	{
		CONFIG_VALUE::iterator subIt = it->second.find(sName);
		if (subIt != it->second.end())
		{
			sValue = subIt->second;
		}
	}

	return sValue;
}

int CConfig::getValue(const std::string& sApp, const std::string& sName,int& iValue, const int iDefault)
{
	iValue = iDefault;
	MAPCONFIG::iterator it = m_mapConfig.find(sApp);
	if (it != m_mapConfig.end())
	{
		CONFIG_VALUE::iterator subIt = it->second.find(sName);
		if (subIt != it->second.end())
		{
			iValue = atoi(subIt->second.c_str());
		}
	}
	return iValue;
}

long CConfig::getValue(const std::string& sApp, const std::string& sName,long& lValue, const long lDefault)
{
	lValue = lDefault;
	MAPCONFIG::iterator it = m_mapConfig.find(sApp);
	if (it != m_mapConfig.end())
	{
		CONFIG_VALUE::iterator subIt = it->second.find(sName);
		if (subIt != it->second.end())
		{
			lValue =  atol(subIt->second.c_str());
		}
	}
	return lValue;
}

unsigned short CConfig::getValue(const std::string& sApp, const std::string& sName, unsigned short& wValue,const unsigned short wDefault)
{
	wValue = wDefault;
	MAPCONFIG::iterator it = m_mapConfig.find(sApp);
	if (it != m_mapConfig.end())
	{
		CONFIG_VALUE::iterator subIt = it->second.find(sName);
		if (subIt != it->second.end())
		{
			wValue =  static_cast<unsigned short>(atol(subIt->second.c_str()));
		}
	}
	return wValue;
}

unsigned char CConfig::getValue(const std::string& sApp, const std::string& sName,unsigned char& cValue, const unsigned char cDefault)
{

    cValue = cDefault;
	MAPCONFIG::iterator it = m_mapConfig.find(sApp);
	if (it != m_mapConfig.end())
	{
		CONFIG_VALUE::iterator subIt = it->second.find(sName);
		if (subIt != it->second.end())
		{
			cValue =  static_cast<unsigned char>(atol(subIt->second.c_str()));
		}
	}
	return cValue;

}



unsigned long CConfig::getValue(const std::string& sApp, const std::string& sName,unsigned long& dwValue,const unsigned long dwDefault)
{
	dwValue = dwDefault;
	MAPCONFIG::iterator it = m_mapConfig.find(sApp);
	if (it != m_mapConfig.end())
	{
		CONFIG_VALUE::iterator subIt = it->second.find(sName);
		if (subIt != it->second.end())
		{

            dwValue = static_cast<unsigned long>(atoll(subIt->second.c_str()));

		}
	}
	return dwValue;
}

bool CConfig::getValue(const std::string& sApp, const std::string& sName,bool& bValue, const bool bDefault /* = false */)
{
	bValue = bDefault;
	MAPCONFIG::iterator it = m_mapConfig.find(sApp);
	if (it != m_mapConfig.end())
	{
		CONFIG_VALUE::iterator subIt = it->second.find(sName);
		if (subIt != it->second.end())
		{
			bValue =  atol(subIt->second.c_str()) == 0 ? false : true;
		}
	}
	return bValue;
}

unsigned int CConfig::getValue(const std::string& sApp, const std::string& sName,unsigned int& uiValue, const unsigned int uiDefault)
{
	uiValue = uiDefault;
	MAPCONFIG::iterator it = m_mapConfig.find(sApp);
	if (it != m_mapConfig.end())
	{
		CONFIG_VALUE::iterator subIt = it->second.find(sName);
		if (subIt != it->second.end())
		{

			uiValue = static_cast<unsigned int>(atoll(subIt->second.c_str()));

		}
	}
	return uiValue;
}

//true: 取值成功，false: 按照默认值
bool CConfig::getIPAndPort(const std::string& sApp,
							const std::string& sName,
							std::string &sIP,
							unsigned short& wPort,
							const std::string &sDefaultIP,
							const unsigned short wDefaultPort)
{
	sIP = "";
	wPort = 0;

	std::string sIPAndPort("");
	getValue(sApp, sName, sIPAndPort, "");
	if(sIPAndPort.size() >= 10)
	{
		std::string::size_type ID = sIPAndPort.find(":");
		if(ID != std::string::npos)
		{
			sIP = sIPAndPort.substr(0, ID);
			wPort = (unsigned short)atoi(sIPAndPort.substr(ID+1).c_str());
		}
	}

	if(sIP.size() >= 7 && wPort > 0)
	{
		return true;
	}
	else
	{
		sIP = sDefaultIP;
		wPort = wDefaultPort;
		return false;
	}
}

//true: 取值成功，false: 按照默认值
bool CConfig::getIPAndPortList(const std::string& sApp,
								const std::string& sName,
								std::vector<std::pair<std::string, unsigned short> >& vecSvrs,
								const std::string &sDefaultIP,
								const unsigned short wDefaultPort)
{
	vecSvrs.clear();

	std::string sServerList("");
	getValue(sApp, sName, sServerList, "");
	if(sServerList.size() >= 10  )
	{
		std::string::size_type id0 = 0, id1 = 0;
		while(1)
		{
			bool bEnd = false;

			std::string sServer;
			if((id1 = sServerList.find("|", id0)) != std::string::npos)
			{
				sServer = sServerList.substr(id0, id1-id0);
				id0 = id1 + 1;
			}
			else
			{
				sServer = sServerList.substr(id0);
				bEnd = true;
			}


			std::string::size_type ID = sServer.find(":");
			if(ID != std::string::npos)
			{
				std::string sIP = sServer.substr(0, ID);
				unsigned short wPort = (unsigned short)atoi(sServer.substr(ID+1).c_str());

				vecSvrs.push_back(std::make_pair(sIP, wPort));
			}

			if(bEnd)
			{
				break;
			}
		}
	}

	if(vecSvrs.size() > 0)
	{
		return true;
	}
	else
	{
		vecSvrs.push_back(std::make_pair(sDefaultIP, wDefaultPort));
		return false;
	}
}


};

