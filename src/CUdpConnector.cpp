#include "CUdpConnector.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stropts.h>
#include <net/if.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>

namespace lce
{

const char* CUdpConnector::getLocalIp()
{
	static char ip[20]={0};
	int i;
	int s = socket (AF_INET, SOCK_DGRAM, 0);

	for (i=1;;i++)
	{
		struct ifreq ifr;
		struct sockaddr_in *sin = (struct sockaddr_in *) &ifr.ifr_addr;

		ifr.ifr_ifindex = i;
		if (ioctl (s, SIOCGIFNAME, &ifr) < 0)
			break;

		/* now ifr.ifr_name is set */
		if (ioctl (s, SIOCGIFADDR, &ifr) < 0)
			continue;

//		memset(ip, 0, sizeof(ip));
		strcpy(ip , inet_ntoa (sin->sin_addr));
		if(0==strncmp(ip, "172", 3) || 0==strncmp(ip, "192", 3) || 0==strncmp(ip, "10.", 3))
		{
			::close (s);
			return ip;
		}
		else
			continue;
	}

	::close (s);
	return NULL;
}


bool CUdpConnector::connect(const std::string& sSvrIp,const unsigned short wSvrPort,const std::string& sLocalIp,const unsigned short wLocalIp)
{
	m_sSvrIp = sSvrIp;
	m_wSvrPort = wSvrPort;

	if ( (m_iFd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Connect: socket error(%d):%s.",errno, strerror(errno));
		return false;
	}

	int reuse = 1;
    setsockopt(m_iFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(int));

	struct sockaddr_in sockClentAddr;
	memset(&sockClentAddr, 0, sizeof(sockaddr_in));    /* zero out */
	sockClentAddr.sin_family      = AF_INET;
	if ( sLocalIp.empty() )
	{
		sockClentAddr.sin_addr.s_addr = inet_addr(getLocalIp());
	}
	else
	{
		sockClentAddr.sin_addr.s_addr = inet_addr(sLocalIp.c_str());
	}
	sockClentAddr.sin_port        = htons(wLocalIp);

	if (bind(m_iFd, (struct sockaddr *) &sockClentAddr, sizeof(sockClentAddr)) < 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"bind error(%d):%s.",errno, strerror(errno));
		this->close();
		return false;
	}

	m_bConnect = true;
	return true;
}

int CUdpConnector::write(const char* pData, const int iSize)
{
	if (!m_bConnect)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"no create Connect.");
		return -1;
	}

	struct sockaddr_in sockSvrAddr;
	memset(&sockSvrAddr, 0, sizeof(sockaddr_in));
	sockSvrAddr.sin_family = AF_INET;
	sockSvrAddr.sin_addr.s_addr = inet_addr(m_sSvrIp.c_str());
	sockSvrAddr.sin_port = htons(m_wSvrPort);

	int iSendLen = 0;
	iSendLen = ::sendto(m_iFd, pData, iSize, 0, (struct sockaddr *)&sockSvrAddr, sizeof(sockSvrAddr));
	if ( iSendLen<0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"sendto error:%s",strerror(errno));
		return false;
	}

	return iSendLen;
}

int CUdpConnector::read(char* pBuf, const int iBufSize, const time_t dwTimeout)
{
	int iRe = 0;
	struct timeval tv;
	tv.tv_sec = dwTimeout/1000;
	tv.tv_usec = (dwTimeout%1000)*1000;
	setsockopt(m_iFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
//	setsockopt(m_iFd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

	int n = ::recvfrom(m_iFd, pBuf, iBufSize, 0, (struct sockaddr *) 0, (socklen_t *) 0);
	if ( n > 0 )
	{
		iRe = n;
	}
	else if (n < 0)
	{
		if (EWOULDBLOCK == n)
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Read recvfrom over time.");
			iRe = -1;
		}
		else
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Read: recvfrom error(%d):%s",errno, strerror(errno));
			iRe = -1;
		}
	}
	else
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Read recvfrom over time.");
		iRe = -1;
	}

	return iRe;
}

int CUdpConnector::read(std::string& sBuf, const time_t dwTimeout)
{
	int iRe = 0;
	int iBufSize = 65535;
	sBuf.resize(iBufSize);

	struct timeval tv;
	tv.tv_sec = dwTimeout/1000;
	tv.tv_usec = (dwTimeout%1000)*1000;
	setsockopt(m_iFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
//	setsockopt(m_iFd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

	int n = ::recvfrom(m_iFd, (char*)sBuf.data(), iBufSize, 0, (struct sockaddr *) 0, (socklen_t *) 0);
	if ( n > 0 )
	{
		sBuf.resize(n);
		iRe = n;
	}
	else if (n < 0)
	{
		sBuf.clear();
		if (EWOULDBLOCK == n)
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Read recvfrom over time.");
			iRe = -1;
		}
		else
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Read: recvfrom error(%d):%s",errno, strerror(errno));
			iRe = -1;
		}
	}
	else
	{
		sBuf.clear();
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Read recvfrom over time.");
		iRe = -1;
	}

	return iRe;
}

void CUdpConnector::close()
{
	::close(m_iFd);
	m_iFd = -1;
	m_bConnect = false;
}

}


