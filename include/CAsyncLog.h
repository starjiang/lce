#ifndef __NCE_ASYNC_LOG_H__
#define __NCE_ASYNC_LOG_H__

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
#include "CThread.h"
#include "CLogBuffer.h"
namespace lce
{

class CAsyncLog:public CThread
{

public:
	~CAsyncLog(void);

	bool init(const std::string& sLogFilePath="",int iLogSecs = 1,const unsigned long dwLogFileMaxSize=1024*1024*10,const unsigned int uiLogFileNum=5,const bool bShowCmd=false,const unsigned long dwBufSize=1024*1024);
	bool write(const std::string& sMsg);
	bool writeRaw(const std::string& sMsg);
	bool write(const char *sFormat, ...);
	void flush();
	const char* getErrMsg() const {	return m_szErrMsg;	}
private:

	int run();

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
	bool writeFile(const char* data,size_t size,const bool bEnd=true);

	bool writeBuffer(const std::string& str, const bool bEnd=true);

private:
	char m_szErrMsg[1024];

	CLogBuffer *m_poWriteBuffer1;
	CLogBuffer *m_poWriteBuffer2;
	CLogBuffer *m_poCurWirteBuffer;
	int m_iCurWriteBufferFlag;

	std::string m_sCurDate;

	int m_iLogSecs;
	std::ofstream m_ofsOutFile;
	std::string m_sLogFilePath;
	std::string m_sLogBasePath;
	unsigned long m_dwLogFileMaxSize;
	unsigned int m_uiLogFileNum;
	bool m_bShowCmd;
	lce::CMutex m_mutex;
};


};

#endif

