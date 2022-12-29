#ifndef __NCE_UTILS_H
#define __NCE_UTILS_H

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/resource.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>

//64bit switch

#if __BYTE_ORDER == __BIG_ENDIAN
	#define ntohll(x)       (x)
	#define htonll(x)       (x)
#else
	#if __BYTE_ORDER == __LITTLE_ENDIAN
		#define ntohll(x)     __bswap_64 (x)
		#define htonll(x)     __bswap_64 (x)
	#endif
#endif


namespace lce
{

    int Bind(int iFd,const std::string &sHost,uint16_t wPort);

    int Listen(int iFd);

	int SetNoneBlock(int iFd);

	int SetNoDelay(int iFd);

	int SetSocketBufSize(int iFd,int iOpt,uint32_t dwBufSize);

	int Close(const int iFd);

	int SetReUseAddr(const int iFd);

	int Send(int iFd, const char *buf, int count);

	int SendTo(int iFd,const char *buf,int count,const std::string &sIp,uint16_t wPort);

    int CreateUdpSock();
    int CreateTcpSock();

    int Connect(int iFd,const std::string &sHost,uint16_t wPort);

	inline uint64_t GetTickCount()
    {
        timeval tv;
        gettimeofday(&tv, 0);
        return tv.tv_sec * 1000 + tv.tv_usec/1000;
    }

	inline std::string GetTimeString()
	{
		time_t t = time(0);
		char ch[64];
		strftime(ch, sizeof(ch), "%Y-%m-%d %H:%M:%S", localtime(&t)); //年-月-日 时-分-秒
		return ch;
	}

    inline std::string InetNtoA(const uint32_t dwIp)
    {
        struct in_addr in;
        in.s_addr = htonl(dwIp);
        return std::string(inet_ntoa(in));
    }

	void InitDaemon();

    bool SetFileLimit(const size_t dwLimit);
	bool SetCoreLimit(const size_t dwLimit);

	std::string GetGMTDate(const time_t& cur);

	time_t Gmt2Time(const char *str);


	std::string FormUrlEncode(const std::string& sSrc);
	std::string FormUrlDecode(const std::string& sSrc);

	static inline std::string charToHex(char c)
	{
		std::string sResult;
		char first, second;

		first = (c & 0xF0) / 16;
		first += first > 9 ? 'A' - 10 : '0';
		second = c & 0x0F;
		second += second > 9 ? 'A' - 10 : '0';

		sResult.append(1, first);
		sResult.append(1, second);

		return sResult;
	}

	static inline char hexToChar(char first, char second)
	{
		int digit;
		digit = (first >= 'A' ? ((first & 0xDF) - 'A') + 10 : (first - '0'));
		digit *= 16;
		digit += (second >= 'A' ? ((second & 0xDF) - 'A') + 10 : (second - '0'));
		return static_cast<char>(digit);
	}

	inline size_t Hash(const char* str,size_t len)
	{
		unsigned long _h = 0;
		for (size_t i=0;i<len;++i)
			_h = 5*_h + str[i];

		return size_t(_h);
	}

	inline size_t Hash(const std::string& s)
	{
        return Hash(s.data(),s.size());
	}

	inline size_t Hash(int v)
    {
        return Hash((char*)&v,sizeof(v));
    }

    inline size_t Hash(unsigned int v)
    {
        return Hash((char*)&v,sizeof(v));
    }

    inline size_t Hash(long v)
    {
        return Hash((char*)&v,sizeof(v));
    }

    inline size_t Hash(unsigned long v)
    {
        return Hash((char*)&v,sizeof(v));
    }

};
#endif
