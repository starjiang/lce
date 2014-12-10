#include "CTcpConnector.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
namespace lce
{


bool CTcpConnector::connect(const std::string& sSvrIp,const unsigned short wSvrPort,const time_t dwTimeout)
{
	m_sSvrIp = sSvrIp;
	m_wSvrPort = wSvrPort;
    m_dwTimeout = dwTimeout;
	return _imp_connect();
}

bool CTcpConnector::_imp_connect()
{
	if ( m_iFd != -1 )
	{
		close();
	}

	m_iFd = socket(PF_INET, SOCK_STREAM, 0);

	int reuse = 1;
    setsockopt(m_iFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(int));


	if ( -1 == m_iFd )
    {
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Connect: socket error(%d):%s.",errno, strerror(errno));
		return false;
	}

	struct sockaddr_in sockSvrAddr;
	memset(&sockSvrAddr, 0, sizeof(sockaddr_in));
	sockSvrAddr.sin_family = AF_INET;
	sockSvrAddr.sin_addr.s_addr = inet_addr(m_sSvrIp.c_str());
	sockSvrAddr.sin_port = htons(m_wSvrPort);

    int flag;
    flag = fcntl(m_iFd,F_GETFL,0);
    flag |= O_NONBLOCK;
    fcntl(m_iFd,F_SETFL,flag);

    int ret = -1;

    ret = ::connect(m_iFd, (struct sockaddr *)&sockSvrAddr, sizeof(struct sockaddr));

    if(ret == 0)
    {
        flag &= ~O_NONBLOCK;
        fcntl(m_iFd,F_SETFL,flag);//设置阻塞
        return true;
    }

    if(errno != EINPROGRESS)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Connect:connect error(%d):%s.",errno, strerror(errno));
        close();
        return false;
    }
    else
    {
        struct timeval tv;

        tv.tv_sec = m_dwTimeout/1000;
        tv.tv_usec = (m_dwTimeout%1000)*1000;

        fd_set set,rset;
        FD_ZERO(&set);
        FD_ZERO(&rset);
        FD_SET(m_iFd,&set);
        FD_SET(m_iFd,&rset);
        ret = select(m_iFd+1,&rset,&set,NULL,&tv);

        if(ret < 0)
        {
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Connect:select error(%d):%s.",errno, strerror(errno));
            close();
            return false;
        }
        else if(ret == 0)
        {
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Connect time out");
            close();
            return false;
        }
        else
        {
            socklen_t ilen = sizeof(int);
            int error = -1;
            getsockopt(m_iFd, SOL_SOCKET, SO_ERROR, &error, &ilen);

            if(error == 0)
            {
                flag &= ~O_NONBLOCK;
                fcntl(m_iFd,F_SETFL,flag);//设置阻塞
                m_bConnect = true;
                return true;
            }
            else
            {
                snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Connect fail");
                close();
                return false;
            }
        }
    }

    snprintf(m_szErrMsg,sizeof(m_szErrMsg),"connect unprocess error");
	return false;
}

int CTcpConnector::write(const char* pData, const int iSize, const time_t dwTimeout)
{
	if (!m_bConnect)
	{
		_imp_connect();
	}

	struct timeval tv;
	tv.tv_sec = dwTimeout/1000;
	tv.tv_usec = (dwTimeout%1000)*1000;
	setsockopt(m_iFd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

	int iSendLen = -1;
	iSendLen = ::send(m_iFd, pData, iSize, 0);
	if ( iSendLen<0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Write: send error:%s.",strerror(errno));
		close();
		return -1;
	}

	return iSendLen;
}

int CTcpConnector::readn(char* pBuf, const int iSize, const time_t dwTimeout)
{
	if (!m_bConnect)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"no create Connect.");
		return -1;
	}

	struct timeval tv;
	tv.tv_sec = dwTimeout/1000;
	tv.tv_usec = (dwTimeout%1000)*1000;
	setsockopt(m_iFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	int iReadSize = 0;
	while (iReadSize < iSize )
	{
		int n = ::recv(m_iFd, pBuf+iReadSize, iSize-iReadSize, 0);
		if (n < 0)
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Read: recv error(%d):%s",errno, strerror(errno));
			return -1;
		}
		else
		{
			if (0 == n)
			{
				this->close();
				return iReadSize;
			}
			iReadSize += n;
		}
	}

	return iReadSize;
}


int CTcpConnector::read(char* pBuf, const int iBufSize, const time_t dwTimeout)
{
	if (!m_bConnect)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Connect fail or no create connect.");
		return -1;
	}

	int iRe = -1;
	struct timeval tv;
	tv.tv_sec = dwTimeout/1000;
	tv.tv_usec = (dwTimeout%1000)*1000;
	setsockopt(m_iFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	int n = ::recv(m_iFd, pBuf, iBufSize, 0);
	if (n < 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Read: recv error(%d):%s",errno, strerror(errno));
		return -1;
	}
	else
	{
		if (0 == n)
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Read: recv error(%d):%s",errno, strerror(errno));
			this->close();
		}
		iRe = n;
	}

	return iRe;
}

int CTcpConnector::read(std::string& sBuf, const time_t dwTimeout)
{
	if (!m_bConnect)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Connect fail or no create connect.");
		return -1;
	}

	int iBufSize = 65535;
	sBuf.resize(iBufSize);

	int iRe = 0;
	struct timeval tv;
	tv.tv_sec = dwTimeout/1000;
	tv.tv_usec = (dwTimeout%1000)*1000;
	setsockopt(m_iFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	int n = ::recv(m_iFd, (char*)sBuf.data(), iBufSize, 0);
	if (n < 0)
	{
		sBuf.clear();
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Read: recv error(%d):%s",errno, strerror(errno));
		return -1;
	}
	else
	{
		if (0 == n)
		{
			this->close();
			sBuf.clear();
		}
		iRe = n;
		sBuf.resize(iRe);
	}

	return iRe;
}


void CTcpConnector::close()
{
	::close(m_iFd);
	m_iFd = -1;
	m_bConnect = false;
}

}

