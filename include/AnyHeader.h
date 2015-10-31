#ifndef __LCE_ANY_HEADER_H
#define __LCE_ANY_HEADER_H

#include <stdint.h>
#include <netinet/in.h>
#include <CAnyValue.h>

namespace lce
{

#pragma pack(1)
/*
	private int len;
	private short ver;
	private int serviceId;
	private int cmd;
	private int seq;
	private int format;
	private long uid;
	private int clientIp;
	private int errno;
	private int flag;
*/

struct AnyHeader
{
public:
    AnyHeader()
    {
        memset(this,0,sizeof(AnyHeader));
        cStx = 0x2;
    }
    void setStx(uint8_t cStx){this->cStx = cStx;}
    uint8_t getStx(){return cStx;}

    void setLen(uint32_t dwLen){this->dwLen = htonl(dwLen);}
    uint32_t getLen(){return ntohl(dwLen);}

    void setVersion(uint16_t wVersion){this->wVersion = htons(wVersion);}
    uint16_t getVersion(){return ntohs(wVersion);}

    void setServiceId(uint32_t dwServiceId){this->dwServiceId = htonl(dwServiceId);}
    uint32_t getServiceId(){return ntohl(dwServiceId);}

    void setCmd(uint32_t dwCmd){this->dwCmd = htonl(dwCmd);}
    uint32_t getCmd(){return ntohl(dwCmd);}

    void setSeq(uint32_t dwSeq){this->dwSeq = htonl(dwSeq);}
    uint32_t getSeq(){return ntohl(dwSeq);}

    void setFormat(uint32_t dwFormat){this->dwFormat = htonl(dwFormat);}
    uint32_t getFormat(){return ntohl(dwFormat);}


    void setUid(uint64_t ddwUid){this->ddwUid = htonl(ddwUid);}
    uint64_t getUid(){return ntohl(ddwUid);}

    void setClientIp(uint32_t dwClientIp){this->dwClientIp = htonll(dwClientIp);}
    uint32_t getClientIp(){return ntohll(dwClientIp);}

    void setErrno(uint32_t dwErrno){this->dwErrno = htonl(dwErrno);}
    uint32_t getErrno(){return ntohl(dwErrno);}

    void setFlag(uint32_t dwFlag){this->dwFlag = htonl(dwFlag);}
    uint32_t getFlag(){return ntohl(dwFlag);}

private:
    uint8_t  cStx;
    uint32_t dwLen;
    uint16_t wVersion;
    uint32_t dwServiceId;
    uint32_t dwCmd;
    uint32_t dwSeq;
    uint32_t dwFormat;
    uint64_t ddwUid;
    uint32_t dwClientIp;
    uint32_t dwErrno;
    uint32_t dwFlag;
};

#pragma pack()

typedef CAnyValuePackage<AnyHeader> CAnyPkg;

}

#endif // __LCE_ANY_HEADER_H
