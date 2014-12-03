#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "CAnyValue.h"

using namespace std;
using namespace lce;

#pragma pack(1)

struct SHead
{
public:
	void setStx(){ m_cStx = 0x2; }
	void setLen(uint32_t dwLen){ m_dwLen = htonl(dwLen); }
	void setCmd(uint16_t wCmd){m_wCmd = htons(wCmd);}
	void setSeq(uint32_t dwSeq){ m_dwSeq = htonl(dwSeq); }
	uint32_t getLen(){ return ntohl(m_dwLen); }
	uint16_t getCmd(){ return ntohs(m_wCmd); }
	uint32_t getSeq(){ return ntohl(m_dwSeq); }

private:
	uint8_t m_cStx;
	uint32_t m_dwLen;
	uint16_t m_wCmd;
	uint32_t m_dwSeq;
};

#pragma pack()


int main(int argc, char *const arvg[]) 
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in srvAddr;
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_addr.s_addr = inet_addr("10.136.170.213");
	srvAddr.sin_port = htons(8001);

	connect(sock, (struct sockaddr *)&srvAddr, sizeof(srvAddr));

	char buff[1024];

	CAnyValuePackage<SHead> oPkg;
	oPkg["name"]="starjiang";
	oPkg["pwd"]="840206";
	oPkg["params"]["xxx"];

	oPkg.head().setStx();
	oPkg.head().setCmd(1001);
	oPkg.encodeJSON();
	oPkg.setEtx();
	oPkg.head().setLen(oPkg.size());

	int helloLen = oPkg.size();

	const char *hello = oPkg.data();

	for (int i = 0; i < 100000000; ++i) 
	{
		int size = 0;
		while (size < helloLen) 
		{
			int ret = write(sock, hello + size, helloLen - size);
			if (ret > 0) size += ret;
		}
		recv(sock, buff, helloLen, MSG_WAITALL);
	}

	close(sock);
	return 0;
}
