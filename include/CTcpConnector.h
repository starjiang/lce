#ifndef __TCP_CONNECTOR_H__
#define __TCP_CONNECTOR_H__

#include <string>

namespace lce
{

	class CTcpConnector
	{
	public:
		CTcpConnector()
			:m_iFd(-1)
			,m_bConnect(false)
		{}
		~CTcpConnector(){
			close();
		}

		bool connect(const std::string& sSvrIp,const unsigned short wSvrPort,const time_t dwTimeout=3000);
		int write(const char* pData, const int iSize, const time_t dwTimeout=3000);
		int read(char* pBuf, const int iBufSize, const time_t dwTimeout=3000);
		int read(std::string& sBuf, const time_t dwTimeout=3000);
		int readn(char* pBuf, const int iSize, const time_t dwTimeout=3000);
		void close();
		const char* getErrMsg() const {	return m_szErrMsg;	}
	private:
		bool _imp_connect();
	private:
		CTcpConnector(const CTcpConnector& rhs);
		CTcpConnector& operator=(const CTcpConnector& rhs);
	private:
		char m_szErrMsg[1024];
		int m_iFd;
		bool m_bConnect;
		std::string m_sSvrIp;
		unsigned short m_wSvrPort;
		time_t m_dwTimeout;
	};

}



#endif

