#ifndef CHTTPWORKER_H
#define CHTTPWORKER_H

#include <string>
#include "Define.h"
#include "CHttpReader.h"
#include "CHttpWriter.h"

using namespace std;

namespace lce
{

class CHttpRequest
{

public:
    CHttpRequest()
    {

    }
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

    CHttpReader & getReader()
    {
        return oHttpReader;
    }

private:

    SSession stSession;
    CHttpReader oHttpReader;
};

class CHttpResponse
{
public:
    CHttpResponse():bClose(false)
    {
    }

    void setSession(const SSession &stSession)
    {
        this->stSession = stSession;
    }

    CHttpWriter & getWriter()
    {
        return oHttpWriter;
    }

    SSession &getSession()
    {
        return stSession;
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
    CHttpWriter oHttpWriter;
    bool bClose;
};

class CHttpWorker
{
public:
    CHttpWorker() {}
    virtual ~CHttpWorker() {}
public:
    virtual void onRequest(CHttpRequest &oRequest,CHttpResponse &oResponse) = 0;

};

};

#endif // CWORKER_H
