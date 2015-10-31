#ifndef CANYWORKER_H
#define CANYWORKER_H

#include <string>
#include "Define.h"
#include "CAnyValue.h"
#include "CAnyPackage.h"
using namespace std;

namespace lce
{

class CAnyRequest
{

public:
    CAnyRequest()
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

    CAnyPackage & getReader()
    {
        return oAnyReader;
    }

private:

    SSession stSession;
    CAnyPackage oAnyReader;
};

class CAnyResponse
{
public:
    CAnyResponse():bClose(false)
    {
    }

    void setSession(const SSession &stSession)
    {
        this->stSession = stSession;
    }

    CAnyPackage & getWriter()
    {
        return oAnyWriter;
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
    CAnyPackage oAnyWriter;
    bool bClose;
};

class CAnyWorker
{
public:
    CAnyWorker() {}
    virtual ~CAnyWorker() {}
public:
    virtual void onRequest(CAnyRequest &oRequest,CAnyResponse &oResponse) = 0;

};

};

#endif // CWORKER_H
