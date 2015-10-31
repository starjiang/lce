#ifndef CWORKER_H
#define CWORKER_H

#include <string>
#include "Define.h"
using namespace std;

namespace lce
{

class CRawRequest
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

    string &getReader()
    {
        return sData;
    }

private:

    SSession stSession;
    string sData;
};

struct CRawResponse
{
public:
    CRawResponse():bClose(false)
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

    string &getWriter()
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

class CRawWorker
{
public:
    CRawWorker() {}
    virtual ~CRawWorker() {}
public:
    virtual void onRequest(CRawRequest &oRequest,CRawResponse &oResponse) = 0;

};

};

#endif // CWORKER_H
