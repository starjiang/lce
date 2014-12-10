#ifndef __UDP_CONNECTOR_H__
#define __UDP_CONNECTOR_H__

#include <string>

#define INVALID_SOCKET -1

namespace lce
{

class CUdpConnector
{
public:
	CUdpConnector()
		:m_iFd(-1)
		,m_bConnect(false)
	{}
	~CUdpConnector(){
		close();
	}

	bool connect(const std::string& sSvrIp,const unsigned short wSvrPort,const std::string& sLocalIp="",const unsigned short wLocalIp=0);
	int write(const char* pData, const int iSize);
	int read(char* pBuf, const int iBufSize, const time_t dwTimeout=3000);
	int read(std::string& sBuf, const time_t dwTimeout=3000);
	void close();
	const char* getErrMsg() const {	return m_szErrMsg;	}
private:
	const char* getLocalIp();
private:
	CUdpConnector(const CUdpConnector& rhs);
	CUdpConnector& operator=(const CUdpConnector& rhs);
private:
	char m_szErrMsg[1024];
	int m_iFd;
	bool m_bConnect;
	std::string m_sSvrIp;
	unsigned short m_wSvrPort;
};

}



#endif


