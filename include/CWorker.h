#ifndef CWORKER_H
#define CWORKER_H

#include <string>
#include "Define.h"
using namespace std;

namespace lce
{

struct SRequest
{

public:
    void setSession(const SSession &stSession)
    {
        this->stSession = stSession;
    }

    SSession &getSession()
    {
        return stSession;
    }

    const SSession &getSession() const
    {
        return stSession;
    }

    void write(const char *szData,int iSize)
    {
        sData.append(szData,iSize);
    }


    void write(const string &sBuf)
    {
        sData.append(sBuf);
    }

    string &getData()
    {
        return sData;
    }

    const string &getData() const
    {
        return sData;
    }

private:

    SSession stSession;
    string sData;
};

struct SResponse
{
public:
    SResponse():bClose(false)
    {

    }

    void setSession(const SSession &stSession)
    {
        this->stSession = stSession;
    }

    void write(const char *szData,int iSize)
    {
        sData.append(szData,iSize);
    }

    void write(const string &sBuf)
    {
        sData.append(sBuf);
    }

    void writeAndClose(const char *szData,int iSize)
    {
        sData.append(szData,iSize);
        bClose = true;
    }

    void writeAndClose(const string &sBuf)
    {
        sData.append(sBuf);
        bClose = true;
    }

    SSession &getSession()
    {
        return stSession;
    }

    string &getData()
    {
        return sData;
    }

    void setCloseFlag(bool bClose)
    {
        this->bClose = bClose;
    }

    bool getCloseFlag()
    {
        return this->bClose;
    }

private:

    SSession stSession;
    string sData;
    bool bClose;
};

class CWorker
{
public:
    CWorker() {}
    virtual ~CWorker() {}
public:
    virtual void onRequest(const SRequest &stRequest,SResponse &stResponse) = 0;

};

};

#endif // CWORKER_H
