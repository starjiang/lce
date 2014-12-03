#include "CCommMgr.h"

namespace lce
{

CCommMgr* CCommMgr::m_pInstance = NULL;

int CCommMgr::createSrv(int iType,const string &sIp,uint16_t wPort,uint32_t dwInitRecvBufLen,uint32_t dwMaxRecvBufLen,uint32_t dwInitSendBufLen,uint32_t dwMaxSendBufLen)
{

    if( iType != SRV_TCP && iType != SRV_UDP )
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,server type error",__FILE__,__LINE__);
        return -1;
    }

	SServerInfo *pstServerInfo=new SServerInfo;

    pstServerInfo->sIp=sIp;
    pstServerInfo->wPort=wPort;
    pstServerInfo->iType=iType;

    pstServerInfo->dwInitRecvBufLen=dwInitRecvBufLen;
    pstServerInfo->dwMaxRecvBufLen=dwMaxRecvBufLen;

    pstServerInfo->dwInitSendBufLen=dwInitSendBufLen;
    pstServerInfo->dwMaxSendBufLen=dwMaxSendBufLen;
	

    if (iType == SRV_TCP)
    {
        int iFd=createTcpSock();
        if(iFd < 0)
        {
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
            delete pstServerInfo;
            return -1;
        }

        lce::setReUseAddr(iFd);
        lce::setNBlock(iFd);

        if(bind(iFd,sIp,wPort) < 0)
        {
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
            lce::close(iFd);
            delete pstServerInfo;
            return -1;
        }

        if(listen(iFd) < 0)
        {
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
            lce::close(iFd);
            delete pstServerInfo;
            return -1;
        }

        pstServerInfo->iFd=iFd;

		if(m_oCEvent.addFdEvent(pstServerInfo->iFd,CEvent::EV_READ,tr1::bind(&CCommMgr::onAccept,this,std::tr1::placeholders::_1,  std::tr1::placeholders::_2),pstServerInfo) < 0 )
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,m_oCEvent.getErrorMsg());
			lce::close(iFd);
			delete pstServerInfo;
			return -1;
		}
    }
    else if(iType == SRV_UDP)
    {
        int iFd=createUdpSock();
        if(iFd < 0)
        {
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
            delete pstServerInfo;
            return -1;
        }
		
		lce::setSocketBufSize(iFd,SO_SNDBUF,256*1024);
		lce::setSocketBufSize(iFd,SO_RCVBUF,256*1024);

        lce::setReUseAddr(iFd);
        lce::setNBlock(iFd);

        if(bind(iFd,sIp,wPort) < 0)
        {
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
            lce::close(iFd);
            delete pstServerInfo;
            return -1;
        }

		pstServerInfo->iFd=iFd;

		if(m_oCEvent.addFdEvent(pstServerInfo->iFd,CEvent::EV_READ,tr1::bind(&CCommMgr::onUdpRead,this,std::tr1::placeholders::_1,  std::tr1::placeholders::_2),pstServerInfo) < 0 )
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,m_oCEvent.getErrorMsg());
			lce::close(iFd);
			delete pstServerInfo;
			return -1;
		}



    }

    pstServerInfo->iSrvId=m_vecServers.size();
    m_vecServers.push_back(pstServerInfo);
    return pstServerInfo->iSrvId;


}

int CCommMgr::createAsyncConn(uint32_t dwInitRecvBufLen,uint32_t dwMaxRecvBufLen,uint32_t dwInitSendBufLen,uint32_t dwMaxSendBufLen)
{

    SServerInfo *pstServerInfo=new SServerInfo;

    pstServerInfo->sIp ="";
    pstServerInfo->wPort = 0;

    pstServerInfo->iType = CONN_TCP;


    pstServerInfo->dwInitRecvBufLen=dwInitRecvBufLen;
    pstServerInfo->dwMaxRecvBufLen=dwMaxRecvBufLen;

    pstServerInfo->dwInitSendBufLen=dwInitSendBufLen;
    pstServerInfo->dwMaxSendBufLen=dwMaxSendBufLen;


    pstServerInfo->iSrvId=m_vecServers.size();
    pstServerInfo->iFd=0;

    m_vecServers.push_back(pstServerInfo);
    return pstServerInfo->iSrvId;


}

int CCommMgr::setProcessor(int iSrvId,CProcessor *pProcessor,int iPkgType)
{

    if( iSrvId <0 || iSrvId >(int) m_vecServers.size()-1 )
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
        return -1;
    }

	if(pProcessor == NULL)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,process pointer is null",__FILE__,__LINE__);
		return -1;
	}

    SServerInfo * pstServerInfo = m_vecServers[iSrvId];

	if (pstServerInfo == NULL)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
		return -1;
	}

	pstServerInfo->pProcessor = pProcessor;

	switch(iPkgType)
	{
	case PKG_RAW:
		pstServerInfo->iPkgType = iPkgType;
		pstServerInfo->pPackageFilter = new CRawPackageFilter;
		break;
	case PKG_HTTP:
		pstServerInfo->iPkgType = iPkgType;
		pstServerInfo->pPackageFilter = new CHttpPackageFilter;
		break;
	case PKG_H2ST3:
		pstServerInfo->iPkgType = iPkgType;
		pstServerInfo->pPackageFilter = new CH2ShortT3PackageFilter;
		break;
	case PKG_H2LT3:
		pstServerInfo->iPkgType = iPkgType;
		pstServerInfo->pPackageFilter = new CH2T3PackageFilter;
		break;
	case PKG_EXT:
		pstServerInfo->iPkgType = iPkgType;
		pstServerInfo->pPackageFilter = NULL;
		break;
	default:
		pstServerInfo->iPkgType = PKG_RAW;
		pstServerInfo->pPackageFilter = new CRawPackageFilter;

	}

    return 0;

}

int CCommMgr::setPkgFilter(int iSrvId,CPackageFilter *pPkgFilter)
{
	if( iSrvId <0 || iSrvId >(int) m_vecServers.size()-1 )
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
		return -1;
	}

	if(pPkgFilter == NULL)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,pkgfilter pointer is null",__FILE__,__LINE__);
		return -1;
	}

	SServerInfo * pstServerInfo=m_vecServers[iSrvId];

	if (pstServerInfo == NULL)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
		return -1;
	}
	pstServerInfo->iPkgType = PKG_EXT;
	pstServerInfo->pPackageFilter = pPkgFilter;
	return 0;
}


void CCommMgr::onWrite(int iFd,void *pData)
{
    if(isClose(iFd))
    {
		return;
    }
    write(iFd);
}


void CCommMgr::onUdpRead(int iFd,void *pData)
{

    SServerInfo *pstServerInfo=(SServerInfo*)pData;
    SClientInfo *pstClientInfo = &m_vecClients[iFd];

	SSession stSession;
	stSession.ddwBeginTime = lce::getTickCount();
	stSession.iFd = iFd;
	stSession.iSvrId = pstServerInfo->iSrvId;


    int iAddrLen = sizeof(struct sockaddr_in);

    pstClientInfo->iSrvId=pstServerInfo->iSrvId;


    if (pstClientInfo->pSocketRecvBuf == NULL)
    {
        pstClientInfo->pSocketRecvBuf=new CSocketBuf(pstServerInfo->dwInitRecvBufLen,pstServerInfo->dwMaxRecvBufLen);
    }

    int iSize = 0;

    if(pstClientInfo->pSocketRecvBuf->getFreeSize() == 0)
    {
        pstClientInfo->pSocketRecvBuf->addFreeBuf();
    }

    if(pstClientInfo->pSocketRecvBuf->getFreeSize() == 0)
    {
        //print error no buf size
        pstClientInfo->pSocketRecvBuf->reset();
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"onUdpRead %s,%d,socket buf no memory",__FILE__,__LINE__);
        if (pstServerInfo->pProcessor != NULL)
            pstServerInfo->pProcessor->onError(stSession,m_szErrMsg,ERR_NO_BUFFER);
        return;
    }

    iSize=::recvfrom(iFd,pstClientInfo->pSocketRecvBuf->getFreeBuf(),pstClientInfo->pSocketRecvBuf->getFreeSize(),0,(struct sockaddr *) &pstClientInfo->stClientAddr,(socklen_t *)&iAddrLen);
   
	stSession.stClientAddr=pstClientInfo->stClientAddr;

    if(iSize > 0 )
    {
        pstClientInfo->pSocketRecvBuf->addData(iSize);

        int iWholePkgFlag = 0;
        int iRealPkgLen = 0;
        int iPkgLen = 0;

        while ( (iWholePkgFlag = pstServerInfo->pPackageFilter->isWholePkg(pstClientInfo->pSocketRecvBuf->getData(), pstClientInfo->pSocketRecvBuf->getSize(), iRealPkgLen, iPkgLen)) == 0 )
        {

            if (pstServerInfo->pProcessor != NULL)
                pstServerInfo->pProcessor->onRead(stSession,pstClientInfo->pSocketRecvBuf->getData(),iPkgLen);

            pstClientInfo->pSocketRecvBuf->reset();
            break;
        }


        if ( -2 == iWholePkgFlag || -1 == iWholePkgFlag)//非法数据包
        {

            pstClientInfo->pSocketRecvBuf->reset();
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"onUdpRead %s,%d,package invalid,errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));
            if (pstServerInfo->pProcessor != NULL)
                pstServerInfo->pProcessor->onError(stSession,m_szErrMsg,ERR_INVALID_PACKAGE);
        }
    }
    else
    {
        pstClientInfo->pSocketRecvBuf->reset();
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"onUdpRead %s,%d,errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));
        if (pstServerInfo->pProcessor != NULL)
            pstServerInfo->pProcessor->onError(stSession,m_szErrMsg,ERR_SOCKET);
    }

}



void CCommMgr::onTcpRead(int iFd,void *pData)
{

    if(isClose(iFd))
    {
        return;
    }

    SClientInfo *pstClientInfo = &m_vecClients[iFd];

    SServerInfo *pstServerInfo = m_vecServers[pstClientInfo->iSrvId];


	SSession stSession;
	stSession.ddwBeginTime=lce::getTickCount();
	stSession.iFd=iFd;
	stSession.iSvrId=pstServerInfo->iSrvId;
	stSession.stClientAddr=pstClientInfo->stClientAddr;

    if (pstClientInfo->pSocketRecvBuf == NULL)
    {
        pstClientInfo->pSocketRecvBuf=new CSocketBuf(pstServerInfo->dwInitRecvBufLen,pstServerInfo->dwMaxRecvBufLen);
    }

    int iSize = 0;

    if(pstClientInfo->pSocketRecvBuf->getFreeSize() == 0)
    {
        pstClientInfo->pSocketRecvBuf->addFreeBuf();
    }

    if(pstClientInfo->pSocketRecvBuf->getFreeSize() == 0)
    {
        //print error no buf size
        close(iFd);
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"onTcpRead %s,%d,socket buf no memory",__FILE__,__LINE__);

		if (pstServerInfo->pProcessor != NULL)
			pstServerInfo->pProcessor->onError(stSession,m_szErrMsg,ERR_NO_BUFFER);

		return;
    }

    iSize=::recv(iFd,pstClientInfo->pSocketRecvBuf->getFreeBuf(),pstClientInfo->pSocketRecvBuf->getFreeSize(),0);


    if(iSize > 0 )
    {
        pstClientInfo->pSocketRecvBuf->addData(iSize);

        int iWholePkgFlag = 0;

        int iRealPkgLen = 0;
        int iPkgLen = 0;

		if(pstServerInfo->pPackageFilter == NULL)
		{
			close(iFd);
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"onTcpRead %s,%d,package filter is null",__FILE__,__LINE__);
			if (pstServerInfo->pProcessor != NULL)
				pstServerInfo->pProcessor->onError(stSession,m_szErrMsg,ERR_PKG_FILTER);
			return;
		}

        while ( (iWholePkgFlag = pstServerInfo->pPackageFilter->isWholePkg(pstClientInfo->pSocketRecvBuf->getData(), pstClientInfo->pSocketRecvBuf->getSize(), iRealPkgLen, iPkgLen)) == 0 )
        {

            if (pstServerInfo->pProcessor != NULL)
                pstServerInfo->pProcessor->onRead(stSession,pstClientInfo->pSocketRecvBuf->getData(),iPkgLen);


            if(isClose(iFd))
                break;

            pstClientInfo->pSocketRecvBuf->removeData(iPkgLen);
        }


        if ( -2 == iWholePkgFlag )//非法数据包
        {
            close(iFd);//非法数据包时，关闭连接
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"onTcpRead %s,%d,package invalid",__FILE__,__LINE__);
            if (pstServerInfo->pProcessor != NULL)
                pstServerInfo->pProcessor->onError(stSession,m_szErrMsg,ERR_INVALID_PACKAGE);
        }
    }
    else if(iSize == 0)
    {

        if (pstServerInfo->pProcessor != NULL)
            pstServerInfo->pProcessor->onClose(stSession);
        close(iFd);
    }
    else
    {

		if( errno == 104) //Connection reset by peer
		{
			if (pstServerInfo->pProcessor != NULL)
				pstServerInfo->pProcessor->onClose(stSession);
			close(iFd);
		}
		else if(errno == EAGAIN || errno == EINTR) //处理连接正常，IO不正常情况，不关闭连接
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"onTcpRead EAGAIN or EINTR %s,%d,errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));
			if (pstServerInfo->pProcessor != NULL)
				pstServerInfo->pProcessor->onError(stSession,m_szErrMsg,ERR_NOT_READY);
		}
		else
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"onTcpRead %s,%d,errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));
			close(iFd);
			if (pstServerInfo->pProcessor != NULL)
				pstServerInfo->pProcessor->onError(stSession,m_szErrMsg,ERR_SOCKET);
		}

    }

}


void CCommMgr::onAccept(int iFd,void *pData)
{

    SServerInfo *pstServerInfo=(SServerInfo *)pData;

	if (pstServerInfo == NULL) return;

	while(true) //循环接受请求，减小epoll软中断次数，提高性能
	{
		struct sockaddr_in stClientAddr;
		int iAddrLen = sizeof(struct sockaddr_in);

		int iClientSock = accept(iFd, (struct sockaddr *) &stClientAddr,(socklen_t *)&iAddrLen);

		SSession stSession;
		stSession.iSvrId=pstServerInfo->iSrvId;

		if (iClientSock < 0)
		{
			if(errno != EAGAIN && errno != EINTR) //Resource temporarily unavailable
			{
				stSession.ddwBeginTime=lce::getTickCount();
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"onAccept %s,%d,accept errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));

				if (pstServerInfo->pProcessor != NULL)
					pstServerInfo->pProcessor->onError(stSession,m_szErrMsg,ERR_SOCKET);
			}
			break;
		}

		stSession.ddwBeginTime=lce::getTickCount();

		if(m_dwClientNum > m_dwMaxClient)
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"onAccept %s,%d,max clients errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));

			if (pstServerInfo->pProcessor != NULL)
				pstServerInfo->pProcessor->onError(stSession,m_szErrMsg,ERR_MAX_CLIENT);

			lce::close(iClientSock);
			continue;
		}

		SClientInfo *pstClientInfo = &m_vecClients[iClientSock];
		
		pstClientInfo->iSrvId=pstServerInfo->iSrvId;
		pstClientInfo->stClientAddr = stClientAddr;
		pstClientInfo->iFd = iClientSock;

		m_dwClientNum++;

		lce::setReUseAddr(iClientSock);
		lce::setNBlock(iClientSock);

		stSession.iFd = iClientSock;
		stSession.stClientAddr = pstClientInfo->stClientAddr;

		if (pstServerInfo->pProcessor) pstServerInfo->pProcessor->onConnect(stSession,true,NULL);

		if(isClose(iClientSock)) continue;

		if(m_oCEvent.addFdEvent(iClientSock,CEvent::EV_READ,tr1::bind(&CCommMgr::onTcpRead,this,std::tr1::placeholders::_1,  std::tr1::placeholders::_2),pstClientInfo) != 0)
		{
			if (pstServerInfo->pProcessor != NULL)
				pstServerInfo->pProcessor->onError(stSession,m_oCEvent.getErrorMsg(),ERR_SOCKET);
			lce::close(iClientSock);
			continue;
		}
	}

}

void CCommMgr::onConnect(int iFd,void *pData)
{

    if(isClose(iFd))
    {
        return;
    }

    m_oCEvent.delFdEvent(iFd,CEvent::EV_WRITE);

	SClientInfo * pstClientInfo = &m_vecClients[iFd];
    SServerInfo * pstServerInfo=m_vecServers[pstClientInfo->iSrvId];

	int error;
	socklen_t ilen = sizeof(int);
	getsockopt(iFd, SOL_SOCKET, SO_ERROR, &error, &ilen);
	
	
    SSession stSession;
    stSession.ddwBeginTime=lce::getTickCount();
    stSession.iFd=iFd;
    stSession.iSvrId=pstServerInfo->iSrvId;
    stSession.stClientAddr=pstClientInfo->stClientAddr;

	if(error == 0)
	{
		if (pstServerInfo->pProcessor != NULL)
			pstServerInfo->pProcessor->onConnect(stSession,true,pData);

		if(!isClose(iFd))
		{
			if(m_oCEvent.addFdEvent(pstClientInfo->iFd,CEvent::EV_READ,tr1::bind(&CCommMgr::onTcpRead,this,std::tr1::placeholders::_1,  std::tr1::placeholders::_2),pstClientInfo) != 0)
			{
				if (pstServerInfo->pProcessor != NULL)
					pstServerInfo->pProcessor->onError(stSession,m_oCEvent.getErrorMsg(),ERR_SOCKET);
			}
		}
	}
	else
	{
		if (pstServerInfo->pProcessor != NULL)
			pstServerInfo->pProcessor->onConnect(stSession,false,pData);
		close(iFd);
	}


}


int CCommMgr::write(const SSession &stSession,const char* pszData, const int iSize,bool bClose)
{

    if(isClose(stSession.iFd))
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,write error maybe client have closed",__FILE__,__LINE__);
        return -1;
    }

    SClientInfo * pstClientInfo = &m_vecClients[stSession.iFd];

    SServerInfo * pstServerInfo=m_vecServers[stSession.iSvrId];


    pstClientInfo->bNeedClose=bClose;
    int iSendBufSize = 0 ;
    int iSendSize = 0;

    if(pstClientInfo->pSocketSendBuf == NULL || pstClientInfo->pSocketSendBuf->getSize() == 0)
    {
        iSendSize=lce::send(stSession.iFd,pszData,iSize);

        if(iSendSize > 0 )
        {
            if (iSendSize < iSize)
            {
                if(pstClientInfo->pSocketSendBuf == NULL)
                {
                    pstClientInfo->pSocketSendBuf=new CSocketBuf(pstServerInfo->dwInitSendBufLen,pstServerInfo->dwMaxSendBufLen);
                }

                if(!pstClientInfo->pSocketSendBuf->addData(pszData+iSendSize,iSize-iSendSize))
				{
					snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,write error buffer less than data",__FILE__,__LINE__);
					return -1;
				}

				if(m_oCEvent.addFdEvent(stSession.iFd,CEvent::EV_WRITE,tr1::bind(&CCommMgr::onWrite,this,std::tr1::placeholders::_1,  std::tr1::placeholders::_2),pstClientInfo) != 0)
				{
					snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s",m_oCEvent.getErrorMsg());
					return -1;
				}
            }
            else
            {
                if(bClose)
                {
                    close(stSession.iFd);
                }
                else
                {
                     m_oCEvent.delFdEvent(stSession.iFd,CEvent::EV_WRITE);
                }
            }
        }
        else
        {
            if( errno == EAGAIN ||errno == EINTR  )
            {
                if(pstClientInfo->pSocketSendBuf == NULL)
                {
                    pstClientInfo->pSocketSendBuf=new CSocketBuf(pstServerInfo->dwInitSendBufLen,pstServerInfo->dwMaxSendBufLen);
                }

                if(!pstClientInfo->pSocketSendBuf->addData(pszData,iSize))
				{
					snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,write error buffer less than data",__FILE__,__LINE__);
					return -1;
				}

                if(m_oCEvent.addFdEvent(stSession.iFd,CEvent::EV_WRITE,tr1::bind(&CCommMgr::onWrite,this,std::tr1::placeholders::_1,  std::tr1::placeholders::_2),pstClientInfo) != 0)
				{
					snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s",m_oCEvent.getErrorMsg());
					return -1;
				}
            }
            else
            {
                snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
                return -1;
            }
        }

    }
    else if((iSendBufSize = pstClientInfo->pSocketSendBuf->getSize()) > 0)
    {

        int iSendSize=lce::send(stSession.iFd,pstClientInfo->pSocketSendBuf->getData(),iSendBufSize);
        if (iSendSize > 0 )
        {
            pstClientInfo->pSocketSendBuf->removeData(iSendSize);
            iSendBufSize -= iSendSize;
        }
        else
        {
            if (errno != EAGAIN && errno != EINTR )
            {
                snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
                return -1;
            }
        }

        if( iSendBufSize > 0)
        {
            if(!pstClientInfo->pSocketSendBuf->addData(pszData,iSize))
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,write error buffer less than data",__FILE__,__LINE__);
				return -1;
			}

			if(m_oCEvent.addFdEvent(stSession.iFd,CEvent::EV_WRITE,tr1::bind(&CCommMgr::onWrite,this,std::tr1::placeholders::_1,  std::tr1::placeholders::_2),pstClientInfo) != 0)
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s",m_oCEvent.getErrorMsg());
				return -1;
			}
        }
        else
        {
            int iSendSize=lce::send(stSession.iFd,pszData,iSize);
            if(iSendSize > 0 )
            {
                if (iSendSize < iSize)
                {
                    if(!pstClientInfo->pSocketSendBuf->addData(pszData+iSendSize,iSize-iSendSize))
					{
						snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,write error buffer less than data",__FILE__,__LINE__);
						return -1;
					}

					if(m_oCEvent.addFdEvent(stSession.iFd,CEvent::EV_WRITE,tr1::bind(&CCommMgr::onWrite,this,std::tr1::placeholders::_1,  std::tr1::placeholders::_2),pstClientInfo) != 0)
					{
						snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s",m_oCEvent.getErrorMsg());
						return -1;
					}
                }
                else
                {
                    if(bClose)
                    {
                        close(stSession.iFd);
                    }
                    else
                    {
                         m_oCEvent.delFdEvent(stSession.iFd,CEvent::EV_WRITE);
                    }
                }
            }
            else
            {
                if( errno == EAGAIN ||errno == EINTR )
                {
                    if(!pstClientInfo->pSocketSendBuf->addData(pszData,iSize))
					{
						snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,write error buffer less than data",__FILE__,__LINE__);
						return -1;
					}

					if(m_oCEvent.addFdEvent(stSession.iFd,CEvent::EV_WRITE,tr1::bind(&CCommMgr::onWrite,this,std::tr1::placeholders::_1,  std::tr1::placeholders::_2),pstClientInfo) != 0)
					{
						snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s",m_oCEvent.getErrorMsg());
						return -1;
					}
                }
                else
                {
                    snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
                    return -1;
                }
            }
        }
    }
    return 0;
}

int CCommMgr::write(int iFd)
{

    SClientInfo * pstClientInfo = &m_vecClients[iFd];
    SServerInfo * pstServerInfo = m_vecServers[pstClientInfo->iSrvId];

	SSession stSession;
	stSession.ddwBeginTime = lce::getTickCount();
	stSession.stClientAddr = pstClientInfo->stClientAddr;
	stSession.iFd = iFd;
	stSession.iSvrId = pstServerInfo->iSrvId;



    int iSendBufSize=pstClientInfo->pSocketSendBuf->getSize();

    if(iSendBufSize > 0)
    {
        int iSendSize=lce::send(iFd,pstClientInfo->pSocketSendBuf->getData(),iSendBufSize);
        if (iSendSize > 0 )
        {
            pstClientInfo->pSocketSendBuf->removeData(iSendSize);
            iSendBufSize -= iSendSize;
        }
        else
        {
            if (errno != EAGAIN && errno != EINTR )
            {
                snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
                close(iFd);
                if (pstServerInfo->pProcessor != NULL)
                    pstServerInfo->pProcessor->onError(stSession,m_szErrMsg,ERR_SOCKET);
                return -1;
            }
        }
    }

    if( iSendBufSize == 0)
    {
        if (pstClientInfo->bNeedClose)
            close(pstClientInfo->iFd);
        else
            m_oCEvent.delFdEvent(iFd,CEvent::EV_WRITE);
    }
    return 0;

}


int CCommMgr::writeTo(const int iSrvId, const string& sIp, const uint16_t wPort,const char* pszData, const int iSize)
{

    if( iSrvId <0 || iSrvId >(int) m_vecServers.size()-1)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
        return -1;
    }


    SServerInfo * pstServerInfo=m_vecServers[iSrvId];

	if (pstServerInfo == NULL)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
		return -1;
	}


    if(lce::sendto(pstServerInfo->iFd,pszData,iSize,sIp,wPort) == -1)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
        return -1;
    }
    return 0;
}


int CCommMgr::close(const SSession & stSession)
{
    int iFd=stSession.iFd;
    return close(iFd);

}

int CCommMgr::close(int iFd)
{
    if( m_vecClients[iFd].iFd != 0)
    {
		SClientInfo *pstClientInfo = &m_vecClients[iFd];

        int iType = m_vecServers[pstClientInfo->iSrvId]->iType;

		if (iType == SRV_TCP) m_dwClientNum--;

        if (iType == SRV_TCP || iType == CONN_TCP)
        {
	        pstClientInfo->iFd = 0;
			
			if(pstClientInfo->pSocketRecvBuf != NULL)
			{
				delete pstClientInfo->pSocketRecvBuf;
				pstClientInfo->pSocketRecvBuf = NULL;
			}

			if(pstClientInfo->pSocketSendBuf != NULL)
			{
				delete pstClientInfo->pSocketSendBuf;
				pstClientInfo->pSocketSendBuf = NULL;
			}	
            m_oCEvent.delFdEvent(iFd,CEvent::EV_READ|CEvent::EV_WRITE);
            
			return lce::close(iFd);
        }

    }
    return 0;
}

inline bool CCommMgr::isClose(int iFd)
{
    return (m_vecClients[iFd].iFd == 0);
}



int CCommMgr::connect(int iSrvId,const string &sIp,uint16_t wPort,void *pData)
{
    if( iSrvId <0 || iSrvId >(int) m_vecServers.size()-1)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
        return -1;
    }

    SServerInfo * pstServerInfo = m_vecServers[iSrvId];

	if (pstServerInfo == NULL)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
		return -1;
	}

    if(pstServerInfo->iType == CONN_TCP)
    {

		int iFd = lce::createTcpSock();

		if (iFd < 0)
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
			return -1;
		}

		m_vecClients[iFd].iSrvId = pstServerInfo->iSrvId;


        lce::setReUseAddr(iFd);
        lce::setNBlock(iFd);

		m_vecClients[iFd].iFd = iFd;
        m_vecClients[iFd].stClientAddr.sin_family=AF_INET;
		m_vecClients[iFd].stClientAddr.sin_port=htons(wPort);
		m_vecClients[iFd].stClientAddr.sin_addr.s_addr = inet_addr(sIp.c_str());
		memset(&(m_vecClients[iFd].stClientAddr.sin_zero),0,8);

        int iRet = lce::connect(iFd,sIp,wPort);

        if(iRet != -1)
        {
            SSession stSession;
            stSession.ddwBeginTime=lce::getTickCount();
            stSession.iFd = iFd;
            stSession.iSvrId=pstServerInfo->iSrvId;
            stSession.stClientAddr = m_vecClients[iFd].stClientAddr;

            if (pstServerInfo->pProcessor != NULL)
                pstServerInfo->pProcessor->onConnect(stSession,true,pData);

            if(isClose(iFd))
            {
                snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,connect error maybe client have closed",__FILE__,__LINE__);
                return -1;
            }
            return iFd;

        }
        else
        {

			if (errno == EINPROGRESS)
            {
				if(m_oCEvent.addFdEvent(iFd,CEvent::EV_WRITE,tr1::bind(&CCommMgr::onConnect,this,std::tr1::placeholders::_1,  std::tr1::placeholders::_2),pData) != 0)
				{
					close(iFd);
					snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,connect error:%s",__FILE__,__LINE__,m_oCEvent.getErrorMsg());
					return -1;
				}
                return iFd;
            }
            else
            {
				close(iFd);
                snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
                return -1;
            }
        }

        return -1;

    }
    else
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,server iType error",__FILE__,__LINE__);
        return -1;
    }

    return -1;
}

int CCommMgr::addTimer(int iTimerId,uint32_t dwExpire,CProcessor *pProcessor,void *pData)
{	
	if(pProcessor == NULL)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,"processor pointor is null");
		return -1;
	}

	return m_oCEvent.addTimer(iTimerId,dwExpire,tr1::bind(&CProcessor::onTimer,pProcessor,std::tr1::placeholders::_1,  std::tr1::placeholders::_2),pData);
}


int CCommMgr::delTimer(int iTimerId)
{
    return m_oCEvent.delTimer(iTimerId);
}

int CCommMgr::addSigHandler(int iSignal,CProcessor *pProcessor)
{
	if(pProcessor == NULL)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,"processor pointor is null");
		return -1;
	}
	m_mapSigProcs[iSignal]=pProcessor;

	signal(iSignal,CCommMgr::onSignal);
    return 0;
}

void CCommMgr::onSignal(int iSignal)
{
	map<int,CProcessor *>::iterator it = CCommMgr::getInstance().m_mapSigProcs.find(iSignal);
	if(it != CCommMgr::getInstance().m_mapSigProcs.end())
	{
		it->second->onSignal(iSignal);
	}
}

int CCommMgr::sendMessage(int iMsgType,CProcessor *pProcessor,void* pData)
{
	if(pProcessor == NULL)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,"processor pointor is null");
		return -1;
	}

	return m_oCEvent.addMessage(iMsgType,tr1::bind(&CProcessor::onMessage,pProcessor,std::tr1::placeholders::_1,  std::tr1::placeholders::_2),pData);

}

int CCommMgr::start()
{
    return  m_oCEvent.run();
}

int CCommMgr::rmSrv(int iSrvId)
{

	if( iSrvId <0 || iSrvId >(int) m_vecServers.size()-1 )
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
		return -1;
	}

	SServerInfo * pstServerInfo=m_vecServers[iSrvId];
	
	if(pstServerInfo == NULL)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
		return -1;
	}

	if(pstServerInfo->iFd > 0)
	{
		m_oCEvent.delFdEvent(pstServerInfo->iFd,CEvent::EV_READ|CEvent::EV_WRITE);
		lce::close(pstServerInfo->iFd);
	}

	delete pstServerInfo;
	
	m_vecServers[iSrvId] = NULL;


	return 0;
}




int CCommMgr::stop()
{
    return m_oCEvent.stop();
}

CCommMgr::~CCommMgr()
{
    m_oCEvent.stop();

	for(vector <SServerInfo *>::iterator it=m_vecServers.begin();it!=m_vecServers.end();++it)
    {
        lce::close((*it)->iFd);
        delete (*it);
    }

}

};