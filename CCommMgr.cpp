#include "CCommMgr.h"

namespace lce
{

CCommMgr* CCommMgr::m_pInstance = NULL;

int CCommMgr::createSrv(int iType,const string &sIp,uint16_t wPort,uint32_t dwInitRecvBufLen,uint32_t dwMaxRecvBufLen,uint32_t dwInitSendBufLen,uint32_t dwMaxSendBufLen,CPackageFilter * pPackageFilter)
{

    if( iType != SRV_TCP && iType != SRV_UDP )
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,server type error",__FILE__,__LINE__);
        return -1;
    }

    SServerInfo *pstServerInfo=new SServerInfo;

    if(pstServerInfo == NULL)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,no memory for server error",__FILE__,__LINE__);
        delete pstServerInfo;
        return -1;
    }

    pstServerInfo->sIp=sIp;
    pstServerInfo->wPort=wPort;

    pstServerInfo->pPackageFilter=pPackageFilter;
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

        lce::setNCloseWait(iFd);
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

        lce::setNCloseWait(iFd);
        lce::setNBlock(iFd);

        if(bind(iFd,sIp,wPort) < 0)
        {
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
            lce::close(iFd);
            delete pstServerInfo;
            return -1;
        }

        pstServerInfo->iFd=iFd;
    }


    pstServerInfo->iSrvId=m_vecServers.size();
    m_vecServers.push_back(pstServerInfo);
    return pstServerInfo->iSrvId;


}

int CCommMgr::createAsyncConn(int iType,uint32_t dwInitRecvBufLen,uint32_t dwMaxRecvBufLen,uint32_t dwInitSendBufLen,uint32_t dwMaxSendBufLen,CPackageFilter * pPackageFilter)
{

    if( iType != CONN_TCP )
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,server type error",__FILE__,__LINE__);
        return -1;
    }

    SServerInfo *pstServerInfo=new SServerInfo;

    if(pstServerInfo == NULL)
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,no memory for server error",__FILE__,__LINE__);
        return -1;
    }


    pstServerInfo->sIp ="";
    pstServerInfo->wPort = 0;


    pstServerInfo->pPackageFilter=pPackageFilter;
    pstServerInfo->iType=iType;


    pstServerInfo->dwInitRecvBufLen=dwInitRecvBufLen;
    pstServerInfo->dwMaxRecvBufLen=dwMaxRecvBufLen;

    pstServerInfo->dwInitSendBufLen=dwInitSendBufLen;
    pstServerInfo->dwMaxSendBufLen=dwMaxSendBufLen;


    pstServerInfo->iSrvId=m_vecServers.size();
    pstServerInfo->iFd=0;

    m_vecServers.push_back(pstServerInfo);
    return pstServerInfo->iSrvId;


}

int CCommMgr::setCallBack(int iSrvId,CommOnRead pOnRead,CommOnConnect pOnConnect,CommOnClose pOnClose,CommOnError pOnError)
{

    if( iSrvId <0 || iSrvId >(int) m_vecServers.size() )
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
        return -1;
    }

    SServerInfo * pstServerInfo=m_vecServers[iSrvId];

    pstServerInfo->pOnRead=pOnRead;
    pstServerInfo->pOnClose=pOnClose;
    pstServerInfo->pOnConnect=pOnConnect;
    pstServerInfo->pOnError=pOnError;
    return 0;

}


void CCommMgr::onWrite(int iFd,void *pData)
{

    if(CCommMgr::getInstance().isClose(iFd))
    {
        return;
    }
    SClientInfo * pstClientInfo = (SClientInfo *)pData;
    CCommMgr::getInstance().write(pstClientInfo->iFd);
}


void CCommMgr::onUdpRead(int iFd,void *pData)
{

    SServerInfo *pstServerInfo=(SServerInfo*)pData;

    SClientInfo *pstClientInfo = NULL;

    if ( CCommMgr::getInstance().m_vecClients[iFd] == NULL)
    {
        pstClientInfo=new SClientInfo;
        if(pstClientInfo == NULL)
        {
            snprintf(CCommMgr::getInstance().m_szErrMsg,sizeof(CCommMgr::getInstance().m_szErrMsg),"%s,%d,no memory for client",__FILE__,__LINE__);
            if (pstServerInfo->pOnError != NULL)
                pstServerInfo->pOnError(CCommMgr::getInstance().m_szErrMsg);
            return;
        }
        CCommMgr::getInstance().m_vecClients[iFd]=pstClientInfo;
    }

    else
        pstClientInfo=CCommMgr::getInstance().m_vecClients[iFd];

    int iAddrLen = sizeof(struct sockaddr_in);

    pstClientInfo->iSrvId=pstServerInfo->iSrvId;


    if (pstClientInfo->pSocketRecvBuf == NULL)
    {
        pstClientInfo->pSocketRecvBuf=new CSocketBuf(pstServerInfo->dwInitRecvBufLen,pstServerInfo->dwMaxRecvBufLen);
        if(pstClientInfo->pSocketRecvBuf == NULL)
        {
            snprintf(CCommMgr::getInstance().m_szErrMsg,sizeof(CCommMgr::getInstance().m_szErrMsg),"%s,%d,no memory for recv buf",__FILE__,__LINE__);
            if (pstServerInfo->pOnError != NULL)
                pstServerInfo->pOnError(CCommMgr::getInstance().m_szErrMsg);
            return;
        }
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
        snprintf(CCommMgr::getInstance().m_szErrMsg,sizeof(CCommMgr::getInstance().m_szErrMsg),"%s,%d,socket buf no memory",__FILE__,__LINE__);
        if (pstServerInfo->pOnError != NULL)
            pstServerInfo->pOnError(CCommMgr::getInstance().m_szErrMsg);
        return;
    }

    iSize=::recvfrom(iFd,pstClientInfo->pSocketRecvBuf->getFreeBuf(),pstClientInfo->pSocketRecvBuf->getFreeSize(),0,(struct sockaddr *) &pstClientInfo->stClientAddr,(socklen_t *)&iAddrLen);

    if(iSize > 0 )
    {
        pstClientInfo->pSocketRecvBuf->addData(iSize);

        int iWholePkgFlag = 0;
        SSession stSession;
        int iRealPkgLen = 0;
        int iPkgLen = 0;

        while ( (iWholePkgFlag = pstServerInfo->pPackageFilter->isWholePkg(pstClientInfo->pSocketRecvBuf->getData(), pstClientInfo->pSocketRecvBuf->getSize(), iRealPkgLen, iPkgLen)) == 0 )
        {

            //const char* pRealPkgData = pstServerInfo->pPackageFilter->getRealPkgData(pstClientInfo->pSocketRecvBuf->getData(), pstClientInfo->pSocketRecvBuf->getSize());

            stSession.dwBeginTime=lce::getTickCount();
            stSession.iFd=iFd;
            stSession.iType=pstServerInfo->iType;
            stSession.iSvrId=pstServerInfo->iSrvId;
            stSession.stClientAddr=pstClientInfo->stClientAddr;

            //pstServerInfo->pOnRead(stSession,pRealPkgData,iRealPkgLen);
            if (pstServerInfo->pOnRead != NULL)
                pstServerInfo->pOnRead(stSession,pstClientInfo->pSocketRecvBuf->getData(),iPkgLen);
            pstClientInfo->pSocketRecvBuf->reset();
            break;
        }


        if ( -2 == iWholePkgFlag || -1 == iWholePkgFlag)//非法数据包
        {
            pstClientInfo->pSocketRecvBuf->reset();
            snprintf(CCommMgr::getInstance().m_szErrMsg,sizeof(CCommMgr::getInstance().m_szErrMsg),"%s,%d,package invalid,errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));
            if (pstServerInfo->pOnError != NULL)
                pstServerInfo->pOnError(CCommMgr::getInstance().m_szErrMsg);
        }
    }
    else
    {
        pstClientInfo->pSocketRecvBuf->reset();
        snprintf(CCommMgr::getInstance().m_szErrMsg,sizeof(CCommMgr::getInstance().m_szErrMsg),"%s,%d,errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));
        if (pstServerInfo->pOnError != NULL)
            pstServerInfo->pOnError(CCommMgr::getInstance().m_szErrMsg);
    }

}



void CCommMgr::onTcpRead(int iFd,void *pData)
{

    if(CCommMgr::getInstance().isClose(iFd))
    {
        return;
    }

    SClientInfo *pstClientInfo=(SClientInfo*)pData;

    SServerInfo *pstServerInfo=CCommMgr::getInstance().m_vecServers[pstClientInfo->iSrvId];

    if (pstClientInfo->pSocketRecvBuf == NULL)
    {
        pstClientInfo->pSocketRecvBuf=new CSocketBuf(pstServerInfo->dwInitRecvBufLen,pstServerInfo->dwMaxRecvBufLen);

        if(pstClientInfo->pSocketRecvBuf == NULL)
        {
            snprintf(CCommMgr::getInstance().m_szErrMsg,sizeof(CCommMgr::getInstance().m_szErrMsg),"%s,%d,no memory for recv buf",__FILE__,__LINE__);
            if (pstServerInfo->pOnError != NULL)
                pstServerInfo->pOnError(CCommMgr::getInstance().m_szErrMsg);
            return;
        }
    }

    int iSize = 0;

    if(pstClientInfo->pSocketRecvBuf->getFreeSize() == 0)
    {
        pstClientInfo->pSocketRecvBuf->addFreeBuf();
    }

    if(pstClientInfo->pSocketRecvBuf->getFreeSize() == 0)
    {
        //print error no buf size
        snprintf(CCommMgr::getInstance().m_szErrMsg,sizeof(CCommMgr::getInstance().m_szErrMsg),"%s,%d,socket buf no memory",__FILE__,__LINE__);
        CCommMgr::getInstance().close(iFd);
        return pstServerInfo->pOnError(CCommMgr::getInstance().m_szErrMsg);

    }

    iSize=::recv(iFd,pstClientInfo->pSocketRecvBuf->getFreeBuf(),pstClientInfo->pSocketRecvBuf->getFreeSize(),0);

    if(iSize > 0 )
    {
        pstClientInfo->pSocketRecvBuf->addData(iSize);

        int iWholePkgFlag = 0;
        SSession stSession;
        int iRealPkgLen = 0;
        int iPkgLen = 0;

        while ( (iWholePkgFlag = pstServerInfo->pPackageFilter->isWholePkg(pstClientInfo->pSocketRecvBuf->getData(), pstClientInfo->pSocketRecvBuf->getSize(), iRealPkgLen, iPkgLen)) == 0 )
        {

            //const char* pRealPkgData = pstServerInfo->pPackageFilter->getRealPkgData(pstClientInfo->pSocketRecvBuf->getData(), pstClientInfo->pSocketRecvBuf->getSize());

            stSession.dwBeginTime=lce::getTickCount();
            stSession.iFd=iFd;
            stSession.iSvrId=pstServerInfo->iSrvId;
            stSession.stClientAddr=pstClientInfo->stClientAddr;
            if (pstServerInfo->pOnRead != NULL)
                pstServerInfo->pOnRead(stSession,pstClientInfo->pSocketRecvBuf->getData(),iPkgLen);

            if(CCommMgr::getInstance().isClose(iFd))
                break;

            pstClientInfo->pSocketRecvBuf->removeData(iPkgLen);
        }


        if ( -2 == iWholePkgFlag )//非法数据包
        {
            CCommMgr::getInstance().close(iFd);
            snprintf(CCommMgr::getInstance().m_szErrMsg,sizeof(CCommMgr::getInstance().m_szErrMsg),"%s,%d,package invalid",__FILE__,__LINE__);
            if (pstServerInfo->pOnError != NULL)
                pstServerInfo->pOnError(CCommMgr::getInstance().m_szErrMsg);
        }
    }
    else if(iSize == 0)
    {


        SSession stSession;
        stSession.dwBeginTime=lce::getTickCount();
        stSession.iFd=iFd;
        stSession.iSvrId=pstServerInfo->iSrvId;
        stSession.stClientAddr=pstClientInfo->stClientAddr;
        if (pstServerInfo->pOnClose != NULL)
            pstServerInfo->pOnClose(stSession);

        CCommMgr::getInstance().close(iFd);



    }
    else
    {
        snprintf(CCommMgr::getInstance().m_szErrMsg,sizeof(CCommMgr::getInstance().m_szErrMsg),"%s,%d,errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));

        CCommMgr::getInstance().close(iFd);
        if (pstServerInfo->pOnClose != NULL)
            pstServerInfo->pOnError(CCommMgr::getInstance().m_szErrMsg);
    }

}


void CCommMgr::onAccept(int iFd,void *pData)
{

    SServerInfo *pstServerInfo=(SServerInfo *)pData;

    SClientInfo *pstClientInfo=new SClientInfo;

    if(pstClientInfo == NULL)
    {
        snprintf(CCommMgr::getInstance().m_szErrMsg,sizeof(CCommMgr::getInstance().m_szErrMsg),"%s,%d,no memory for client",__FILE__,__LINE__);
        if (pstServerInfo->pOnError != NULL)
            pstServerInfo->pOnError(CCommMgr::getInstance().m_szErrMsg);
        return;
    }


    int iAddrLen = sizeof(struct sockaddr_in);

    pstClientInfo->iSrvId=pstServerInfo->iSrvId;


    int iClientSock=accept(iFd, (struct sockaddr *) &pstClientInfo->stClientAddr,(socklen_t *)&iAddrLen);

    if (iClientSock < 0)
    {
        snprintf(CCommMgr::getInstance().m_szErrMsg,sizeof(CCommMgr::getInstance().m_szErrMsg),"%s,%d,accept errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));
        pstServerInfo->pOnError(CCommMgr::getInstance().m_szErrMsg);
        lce::close(iClientSock);
        delete pstClientInfo;
        return;
    }
    pstClientInfo->iFd=iClientSock;

    lce::setNCloseWait(iClientSock);
    lce::setNBlock(iClientSock);


    CCommMgr::getInstance().m_vecClients[iClientSock]=pstClientInfo;
    CCommMgr::getInstance().m_oCEvent.addFdEvent(iClientSock,CEvent::EV_READ,CCommMgr::onTcpRead,pstClientInfo);

}

void CCommMgr::onConnect(int iFd,void *pData)
{

    if(CCommMgr::getInstance().isClose(iFd))
    {
        return;
    }

    CCommMgr::getInstance().m_oCEvent.delFdEvent(iFd,CEvent::EV_WRITE);

    SClientInfo * pstClientInfo=(SClientInfo*)pData;
    SServerInfo * pstServerInfo=CCommMgr::getInstance().m_vecServers[pstClientInfo->iSrvId];

    SSession stSession;
    stSession.dwBeginTime=lce::getTickCount();
    stSession.iFd=iFd;
    stSession.iSvrId=pstServerInfo->iSrvId;
    stSession.stClientAddr=pstClientInfo->stClientAddr;


    char szBuf[1];
    int iSize=::recv(iFd,szBuf,1,0);

    if(iSize < 0 && errno == 11)
    {

        CCommMgr::getInstance().m_oCEvent.addFdEvent(pstClientInfo->iFd,CEvent::EV_READ,CCommMgr::onTcpRead,pstClientInfo);
        if (pstServerInfo->pOnConnect != NULL)
            pstServerInfo->pOnConnect(stSession,true);

    }
    else if(iSize == 1)
    {
        if(pstClientInfo->pSocketRecvBuf == NULL)
        {
            pstClientInfo->pSocketRecvBuf=new CSocketBuf(pstServerInfo->dwInitRecvBufLen,pstServerInfo->dwMaxRecvBufLen);

            if(pstClientInfo->pSocketRecvBuf == NULL)
            {
                snprintf(CCommMgr::getInstance().m_szErrMsg,sizeof(CCommMgr::getInstance().m_szErrMsg),"%s,%d,no memory for recv buf",__FILE__,__LINE__);
                if (pstServerInfo->pOnError != NULL)
                    pstServerInfo->pOnError(CCommMgr::getInstance().m_szErrMsg);
                return;
            }
            pstClientInfo->pSocketRecvBuf->addData(szBuf,1);
        }
        CCommMgr::getInstance().m_oCEvent.addFdEvent(pstClientInfo->iFd,CEvent::EV_READ,CCommMgr::onTcpRead,pstClientInfo);
        if (pstServerInfo->pOnConnect != NULL)
            pstServerInfo->pOnConnect(stSession,true);


    }
    else
    {
        if (pstServerInfo->pOnConnect != NULL)
            pstServerInfo->pOnConnect(stSession,false);
        CCommMgr::getInstance().close(iFd);
    }
}


int CCommMgr::write(const SSession &stSession,const char* pszData, const int iSize,bool bClose)
{

    if(CCommMgr::getInstance().isClose(stSession.iFd))
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,write error maybe client have closed",__FILE__,__LINE__);
        return -1;
    }

    SClientInfo * pstClientInfo=m_vecClients[stSession.iFd];

    SServerInfo * pstServerInfo=m_vecServers[stSession.iSvrId];

    pstClientInfo->bNeedClose=bClose;

    if(pstClientInfo->pSocketSendBuf == NULL)
    {
        pstClientInfo->pSocketSendBuf=new CSocketBuf(pstServerInfo->dwInitSendBufLen,pstServerInfo->dwMaxSendBufLen);
        if(pstClientInfo->pSocketSendBuf == NULL)
        {
            snprintf(CCommMgr::getInstance().m_szErrMsg,sizeof(CCommMgr::getInstance().m_szErrMsg),"%s,%d,no memory for send buf",__FILE__,__LINE__);
            return -1;
        }

    }

    int iSendBufSize=pstClientInfo->pSocketSendBuf->getSize();

    if(iSendBufSize > 0)
    {
        int iSendSize=lce::send(stSession.iFd,pstClientInfo->pSocketSendBuf->getData(),iSendBufSize);
        if (iSendSize > 0 )
        {
            pstClientInfo->pSocketSendBuf->removeData(iSendSize);
            iSendBufSize -= iSendSize;
        }
        else
        {
            if (errno != EAGAIN)
            {
                snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
                close(stSession.iFd);
                return -1;
            }
        }
    }

    if( iSendBufSize > 0)
    {
        pstClientInfo->pSocketSendBuf->addData(pszData,iSize);
        m_oCEvent.addFdEvent(stSession.iFd,CEvent::EV_WRITE,CCommMgr::onWrite,pstClientInfo);
    }
    else
    {
        int iSendSize=lce::send(stSession.iFd,pszData,iSize);
        if(iSendSize > 0 )
        {
            if (iSendSize < iSize)
            {
                pstClientInfo->pSocketSendBuf->addData(pszData+iSendSize,iSize-iSendSize);
                m_oCEvent.addFdEvent(stSession.iFd,CEvent::EV_WRITE,CCommMgr::onWrite,pstClientInfo);
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
            if( errno == EAGAIN )
            {
                pstClientInfo->pSocketSendBuf->addData(pszData,iSize);
                m_oCEvent.addFdEvent(stSession.iFd,CEvent::EV_WRITE,CCommMgr::onWrite,pstClientInfo);
            }
            else
            {
                snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
                close(stSession.iFd);
                return -1;
            }
        }
    }
    return 0;
}

int CCommMgr::write(int iFd)
{
    if(CCommMgr::getInstance().isClose(iFd))
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,write error maybe client have closed",__FILE__,__LINE__);
        return -1;
    }

    SClientInfo * pstClientInfo=m_vecClients[iFd];

    SServerInfo * pstServerInfo=m_vecServers[pstClientInfo->iSrvId];

    if(pstClientInfo->pSocketSendBuf == NULL)
    {
        pstClientInfo->pSocketSendBuf=new CSocketBuf(pstServerInfo->dwInitSendBufLen,pstServerInfo->dwMaxSendBufLen);
        if(pstClientInfo->pSocketSendBuf == NULL)
        {
            snprintf(CCommMgr::getInstance().m_szErrMsg,sizeof(CCommMgr::getInstance().m_szErrMsg),"%s,%d,no memory for send buf",__FILE__,__LINE__);

            if (pstServerInfo->pOnError != NULL)
                pstServerInfo->pOnError(CCommMgr::getInstance().m_szErrMsg);
            return -1;
        }
    }

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
            if (errno != EAGAIN)
            {
                snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
                close(iFd);
                if (pstServerInfo->pOnError != NULL)
                    pstServerInfo->pOnError(CCommMgr::getInstance().m_szErrMsg);
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


int CCommMgr::writeTo(const int iSrvId, const string& sIp, const uint16_t wPort, const char* pszData, const int iSize)
{

    if( iSrvId <0 || iSrvId >(int) m_vecServers.size() )
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
        return -1;
    }


    SServerInfo * pstServerInfo=m_vecServers[iSrvId];

    if(lce::sendto(pstServerInfo->iFd,pszData,iSize,sIp,wPort)== -1)
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
    if( m_vecClients[iFd] != NULL)
    {
        int iType= m_vecServers[m_vecClients[iFd]->iSrvId]->iType;

        if (iType == SRV_TCP || iType == CONN_TCP)
        {
            delete m_vecClients[iFd];
            m_vecClients[iFd]=NULL;
            m_oCEvent.delFdEvent(iFd,CEvent::EV_READ|CEvent::EV_WRITE);
            return lce::close(iFd);
        }

    }
    return 0;
}

inline bool CCommMgr::isClose(int iFd)
{
    return (m_vecClients[iFd] == NULL);
}



int CCommMgr::connect(int iSrvId,const string &sIp,uint16_t wPort)
{
    if( iSrvId <0 || iSrvId >(int) m_vecServers.size() )
    {
        snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
        return -1;
    }

    SServerInfo * pstServerInfo=m_vecServers[iSrvId];


    if(pstServerInfo->iType == CONN_TCP)
    {

        SClientInfo * pstClientInfo=new SClientInfo;
        pstClientInfo->iFd=lce::createTcpSock();
        pstClientInfo->iSrvId=pstServerInfo->iSrvId;

        if (pstClientInfo->iFd < 0)
        {
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
            delete pstClientInfo;
            return -1;

        }

        lce::setNCloseWait(pstClientInfo->iFd);
        lce::setNBlock(pstClientInfo->iFd);


        pstClientInfo->stClientAddr.sin_family=AF_INET;
		pstClientInfo->stClientAddr.sin_port=htons(wPort);
		pstClientInfo->stClientAddr.sin_addr.s_addr = inet_addr(sIp.c_str());
		memset(&(pstClientInfo->stClientAddr.sin_zero),0,8);

        int iRet=lce::connect(pstClientInfo->iFd,sIp,wPort);

        if(iRet != -1)
        {
            SSession stSession;
            stSession.dwBeginTime=lce::getTickCount();
            stSession.iFd=pstClientInfo->iFd;
            stSession.iSvrId=pstServerInfo->iSrvId;
            stSession.stClientAddr=pstClientInfo->stClientAddr;
            m_vecClients[pstClientInfo->iFd]=pstClientInfo;
            if (pstServerInfo->pOnConnect != NULL)
                pstServerInfo->pOnConnect(stSession,true);

            if(CCommMgr::getInstance().isClose(pstClientInfo->iFd))
            {
                snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,connect error maybe client have closed",__FILE__,__LINE__);
                return -1;
            }
            return pstClientInfo->iFd;


        }
        else
        {
            if (errno == EINPROGRESS)
            {
                m_oCEvent.addFdEvent(pstClientInfo->iFd,CEvent::EV_WRITE,CCommMgr::onConnect,pstClientInfo);
                m_vecClients[pstClientInfo->iFd]=pstClientInfo;
                return pstClientInfo->iFd;
            }
            else
            {
                snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
                delete pstClientInfo;
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

int CCommMgr::addTimer(uint32_t dwTimerId,uint32_t dwExpire,CommOnTimer pCallBack,void *pData)
{
    return m_oCEvent.addTimer(dwTimerId,dwExpire,pCallBack,pData);
}

int CCommMgr::delTimer(uint32_t dwTimerId)
{
    return m_oCEvent.delTimer(dwTimerId);
}

int CCommMgr::addSigHandler(int iSignal,CommOnSignal pCallBack)
{
    signal(iSignal,pCallBack);
    return 0;
}

int CCommMgr::sendMessage(uint32_t dwMsgType,CommOnMessage pCallBack,void* pData)
{
    return m_oCEvent.addMessage(dwMsgType,pCallBack,pData);
}


int CCommMgr::start()
{

    for(vector<SServerInfo*>::iterator it=m_vecServers.begin();it!=m_vecServers.end();++it)
    {
        if((*it)->iType == SRV_TCP && (*it)->iFd > 0 )
        {
            if(m_oCEvent.addFdEvent((*it)->iFd,CEvent::EV_READ,CCommMgr::onAccept,(*it)) < 0 )
            {
                snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,m_oCEvent.getErrorMsg());
                return -1;
            }
        }
        else if((*it)->iType == SRV_UDP  && (*it)->iFd > 0 )
        {
            if(m_oCEvent.addFdEvent((*it)->iFd,CEvent::EV_READ,CCommMgr::onUdpRead,(*it)) < 0 )
            {
                snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,m_oCEvent.getErrorMsg());
                return -1;
            }
        }
    }

    return  m_oCEvent.run();

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

    for(vector <SClientInfo *>::iterator it=m_vecClients.begin();it!=m_vecClients.end();++it)
    {
        if((*it) != NULL)
        {
            lce::close((*it)->iFd);
            delete (*it);
        }
    }

}

};
