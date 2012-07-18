#include "CFileLog.h"


namespace lce
{


CFileLog::CFileLog(const std::string& sLogFilePath,const unsigned long dwLogFileMaxSize, const unsigned int uiLogFileNum, const bool bShowCmd)
	:m_sLogFilePath(sLogFilePath+".log"),
	m_sLogBasePath(sLogFilePath),
	m_dwLogFileMaxSize(dwLogFileMaxSize),
	m_uiLogFileNum(uiLogFileNum),
	m_bShowCmd(bShowCmd)
{

}

CFileLog::~CFileLog()
{
	if (m_ofsOutFile.is_open())
	{
		m_ofsOutFile.close();
	}
}

bool CFileLog::init(const std::string &sLogFilePath, const unsigned long dwLogFileMaxSize, const unsigned int uiLogFileNum, const bool bShowCmd)
{

	m_sLogFilePath=sLogFilePath+".log",
	m_sLogBasePath = sLogFilePath;
	m_dwLogFileMaxSize = dwLogFileMaxSize;
	m_uiLogFileNum = uiLogFileNum;
	m_bShowCmd = bShowCmd;

	return true;
}

bool CFileLog::shiftFiles()
{
	size_t dwFileSize=this->getFileSize(m_sLogFilePath);
	if (dwFileSize>=m_dwLogFileMaxSize)
	{
		if (m_ofsOutFile.is_open())
		{
			m_ofsOutFile.close();
		}

		char szLogFileName[1024];
		char szNewLogFileName[1024];

		snprintf(szLogFileName,sizeof(szLogFileName),"%s%u.log",m_sLogBasePath.c_str(),m_uiLogFileNum-1);
		if (access(szLogFileName, F_OK) == 0)
		{
			if (remove(szLogFileName) < 0 )
			{
				snprintf(m_szErrMsg, sizeof(m_szErrMsg),"remove error: errno=%d.",errno);
				return false;
			}
		}

		for(int i=m_uiLogFileNum-2; i>=0; i--)
		{
			if (i == 0)
				snprintf(szLogFileName,sizeof(szLogFileName),"%s.log",m_sLogBasePath.c_str());
			else
				snprintf(szLogFileName,sizeof(szLogFileName),"%s%u.log",m_sLogBasePath.c_str(),i);

			if (access(szLogFileName, F_OK) == 0)
			{
				snprintf(szNewLogFileName,sizeof(szNewLogFileName),"%s%d.log",m_sLogBasePath.c_str(),i+1);
				if (rename(szLogFileName,szNewLogFileName) < 0 )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg),"rename error:errno=%d.",errno);
					return false;
				}
			}
		}
	}

	return true;
}

bool CFileLog::writeFile(const std::string &str, const bool bEnd)
{

	if (m_bShowCmd)
	{
		std::cout << str << std::endl;
	}

	if (!m_ofsOutFile.is_open())
	{
		m_ofsOutFile.open(m_sLogFilePath.c_str(),std::ios::out|std::ios::app);
	}

	if(m_ofsOutFile.fail())
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"open file<%s> err!", m_sLogFilePath.c_str());
		return false;
	}

	if (!shiftFiles())
	{
		return false;
	}

	if (!m_ofsOutFile.is_open())
	{
		m_ofsOutFile.open(m_sLogFilePath.c_str(),std::ios::out|std::ios::app);
	}

	if(m_ofsOutFile.fail())
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"open file<%s> err!", m_sLogFilePath.c_str());
		return false;
	}

	m_ofsOutFile << str;
	if (bEnd)
	{
		m_ofsOutFile << std::endl;
	}

	return true;
}



bool CFileLog::write(const char *sFormat, ...)
{

	lce::CAutoLock autoLock(m_mutex);
	char szTemp[10240];
	va_list ap;

	va_start(ap, sFormat);
	memset(szTemp, 0, sizeof(szTemp));
	vsnprintf(szTemp,sizeof(szTemp),sFormat, ap);
	va_end(ap);
	std::string sLog;
	sLog = "[" + getDateTime() + "]:";
	sLog += szTemp;

	if(!writeFile(sLog))
		return false;


	return true;
}

bool CFileLog::write(const std::string& sMsg)
{

	lce::CAutoLock autoLock(m_mutex);

	if(!writeFile("[" + getDateTime() + "]" + sMsg))
		return false;

	return true;
}

bool CFileLog::writeRaw(const std::string& sMsg)
{

	lce::CAutoLock autoLock(m_mutex);

	if(!writeFile(sMsg))
		return false;

	return true;
}


};
