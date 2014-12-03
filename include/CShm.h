#ifndef __NCE_SHM_H__
#define __NCE_SHM_H__

#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>

namespace lce
{

class CShm
{
public:
	CShm(void){
		memset(m_szErrMsg, 0,sizeof(m_szErrMsg));
		m_bCreate = false;
		m_iShmKey = 0 ;
		m_dwShmSize = 0;
		m_pShmBuf = NULL;
		m_iShmID = 0;

	}
	~CShm(void){
		if(!detach())
		{
			printf("%s",m_szErrMsg);
		}
	}

	inline bool create(const int iKey, const unsigned long dwSize, const bool bCreate =true ,const bool bReadOnly=false);
	void* getShmBuf() const {	return m_pShmBuf;	}
	bool isCreate() const {	return m_bCreate;	}
	int getShmKey() const {	return m_iShmKey;	}
	unsigned long getShmSize() const {	return m_dwShmSize;	}
	const char* getErrMsg() const {	return m_szErrMsg;	}

	inline bool detach();

	inline bool attach()
	{
		m_bCreate = false;
		if ((m_pShmBuf = (shmat(m_iShmID, NULL ,0))) == (char*)-1)
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"shmat error:%s", strerror(errno));
			return false;
		}
		return true;
	}

	int getShmID() const {	return m_iShmID;}

	CShm(const CShm& rhs)
	{
		m_bCreate = rhs.m_bCreate;
		m_iShmKey = rhs.m_iShmKey;
		m_dwShmSize = rhs.m_dwShmSize;
		m_pShmBuf = rhs.m_pShmBuf;
		memcpy(m_szErrMsg, rhs.m_szErrMsg, sizeof(m_szErrMsg));
		m_iShmID = rhs.m_iShmID;
	}
	CShm& operator=(const CShm& rhs)
	{
		if ( this != &rhs )
		{
			m_bCreate = rhs.m_bCreate;
			m_iShmKey = rhs.m_iShmKey;
			m_dwShmSize = rhs.m_dwShmSize;
			m_pShmBuf = rhs.m_pShmBuf;
			memcpy(m_szErrMsg, rhs.m_szErrMsg, sizeof(m_szErrMsg));
			m_iShmID = rhs.m_iShmID;

		}

		return *this;
	}
private:
	bool getShm();
	bool remove();
private:
	char m_szErrMsg[256];
	bool m_bCreate;

	int m_iShmKey;
	unsigned long m_dwShmSize;
	void* m_pShmBuf;
	int m_iShmID;
};

bool CShm::detach()
{
	if ( m_pShmBuf != NULL )
	{
		if (shmdt(m_pShmBuf) < 0)
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"shmdt error:%s", strerror(errno));
			return false;
		}
		else
		{
			m_pShmBuf = NULL;
		}
	}
	return true;
}


bool CShm::create(const int iKey, const unsigned long dwSize, const bool bCreate /* =true  */ ,const bool bReadOnly/* = false*/)
{
	m_bCreate = false;

	int iFlag = 0666;
	if (bCreate)
	{
		iFlag = 0666 | IPC_CREAT;
	}
	else
	{
		iFlag = 0666;
	}

	if ((m_iShmID = shmget(iKey, dwSize, 0666)) < 0)
	{
		if ((m_iShmID = shmget(iKey, dwSize, iFlag)) < 0)
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"shmget error:%s", strerror(errno));
			return false;
		}

		m_bCreate = true;
	}

	if ((m_pShmBuf = (shmat(m_iShmID, NULL ,bReadOnly ? SHM_RDONLY : 0 ))) == (char*)-1)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"shmat<m_iShmID=%d> error:%s", m_iShmID, strerror(errno));
		return false;
	}

	m_iShmKey = iKey;
	m_dwShmSize = dwSize;
	return true;
}


};

#endif

