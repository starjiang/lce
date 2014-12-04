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

    void writeData(const char *szData,int iSize)
    {
        sData.append(szData,iSize);
    }


    void writeData(const string &sBuf)
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
    void setSession(const SSession &stSession)
    {
        this->stSession = stSession;
    }

    void writeData(const char *szData,int iSize)
    {
        sData.append(szData,iSize);
    }

    void writeData(const string &sBuf)
    {
        sData.append(sBuf);
    }

    const SSession &getSession() const
    {
        return stSession;
    }

    const string &getData() const
    {
        return sData;
    }

private:

    SSession stSession;
    string sData;
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
