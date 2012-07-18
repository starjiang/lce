#ifndef __NCE_CLOG_H
#define __NCE_CLOG_H

#include "CFileLog.h"
#include <string>
using namespace std;


#define LOG_DEBUG(format, ...) CLog::log(CLog::LEVEL_DEBUG,__FILE__, __LINE__, format, ## __VA_ARGS__)
#define LOG_INFO(format, ...) CLog::log(CLog::LEVEL_INFO,__FILE__, __LINE__, format, ## __VA_ARGS__)
#define LOG_WARN(format, ...) CLog::log(CLog::LEVEL_WARN,__FILE__, __LINE__, format, ## __VA_ARGS__)
#define LOG_ERROR(format, ...) CLog::log(CLog::LEVEL_ERROR,__FILE__, __LINE__, format, ## __VA_ARGS__)

namespace lce
{

class CLog
{
public:

enum{

    LEVEL_DEBUG=1,
    LEVEL_INFO =2,
    LEVEL_WARN =3,
    LEVEL_ERROR=4,
};

public:
    static int init(const string &sLogFile,uint32_t dwLogSize,uint32_t dwLogCount,bool bShowCmd=true,uint32_t cLevel=15,bool bShowLine=true)
    {
        CLog::m_cLevel=cLevel;
        CLog::m_bShowLine=bShowLine;
        return CLog::m_oLog.init(sLogFile,dwLogSize,dwLogCount,bShowCmd);
    }

    static inline void log(const uint8_t cLogLevel,const char* pszFile,const long lLine,const char *sFormat, ...)
    {

        if ((CLog::m_cLevel & cLogLevel) != cLogLevel)
            return;

        char szTemp[4000];
        va_list ap;

        va_start(ap, sFormat);
        memset(szTemp, 0, sizeof(szTemp));

        vsnprintf(szTemp,sizeof(szTemp),sFormat, ap);
        va_end(ap);

        switch (cLogLevel)
        {
        case LEVEL_DEBUG:
            if(CLog::m_bShowLine)
                CLog::m_oLog.write("[DEBUG] %s [file:%s][line:%ld]",szTemp, pszFile, lLine);
            else
                CLog::m_oLog.write("[DEBUG] %s",szTemp, pszFile, lLine);
            break;
        case LEVEL_INFO:
            if(CLog::m_bShowLine)
                CLog::m_oLog.write("[INFO] %s [file:%s][line:%ld]",szTemp, pszFile, lLine);
            else
                CLog::m_oLog.write("[INFO] %s",szTemp, pszFile, lLine);
            break;
        case LEVEL_WARN:
            if(CLog::m_bShowLine)
                CLog::m_oLog.write("[WARN] %s [file:%s][line:%ld]",szTemp, pszFile, lLine);
            else
                CLog::m_oLog.write("[WARN] %s",szTemp, pszFile, lLine);
            break;
        case LEVEL_ERROR:
            if(CLog::m_bShowLine)
                CLog::m_oLog.write("[ERROR] %s [file:%s][line:%ld]",szTemp, pszFile, lLine);
            else
                CLog::m_oLog.write("[ERROR] %s",szTemp, pszFile, lLine);
            break;
        default:
            break;
        }
    }

private:
    static lce::CFileLog m_oLog;
    static uint8_t m_cLevel;
    static bool m_bShowLine;

};

};

#endif
