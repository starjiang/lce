#ifndef __NCE_FILE_LOG_H__
#define __NCE_FILE_LOG_H__

#include "CLock.h"
#include <string>
#include <fstream>
#include <sstream>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include "Utils.h"

namespace lce
{

class CFileLog
{

public:
	~CFileLog();

	bool init(const std::string& sLogFilePath,
			const unsigned long dwLogFileMaxSize=1000000,
			const unsigned int uiLogFileNum=5,
			const bool bShowCmd=false);

	template<class T>
		std::ofstream& operator<<(const T& t)
	{

		lce::CAutoLock autoLock(m_mutex);
		std::stringstream sstr;
		sstr << t;
		std::string sLog;
		sLog = "[" + getDateTime() + "]:";

		writeFile(sLog+sstr.str(), false);
		return m_ofsOutFile;

	}

	bool write(const std::string& sMsg);
	bool writeRaw(const std::string& sMsg);
	bool write(const char *sFormat, ...);
	const char* getErrMsg() const {	return m_szErrMsg;	}
private:
	size_t getFileSize(const std::string& sFile){

		struct stat stStat;
		if (stat(sFile.c_str(), &stStat) >= 0)
		{
			return stStat.st_size;
		}

		return 0;
	}

	std::string getDateTime(){
		time_t	iCurTime;
		time(&iCurTime);
		struct tm curr;
		curr = *localtime(&iCurTime);
		char szDate[50];
		snprintf(szDate, sizeof(szDate), "%04d-%02d-%02d %02d:%02d:%02d", curr.tm_year+1900, curr.tm_mon+1, curr.tm_mday, curr.tm_hour, curr.tm_min, curr.tm_sec);
		return std::string(szDate);
	}

	std::string getDate(){
		time_t	iCurTime;
		time(&iCurTime);
		struct tm curr;
		curr = *localtime(&iCurTime);
		char szDate[50];
		snprintf(szDate, sizeof(szDate), "%04d-%02d-%02d", curr.tm_year+1900, curr.tm_mon+1, curr.tm_mday);
		return std::string(szDate);
	}

	bool shiftFiles();
	bool writeFile(const std::string& str, const bool bEnd=true);
private:
	char m_szErrMsg[1024];
	std::ofstream m_ofsOutFile;
	std::string m_sLogFilePath;
	std::string m_sLogBasePath;
	unsigned long m_dwLogFileMaxSize;
	unsigned int m_uiLogFileNum;
	bool m_bShowCmd;
	lce::CMutex m_mutex;
	std::string m_sCurDate;

};


};

#endif

