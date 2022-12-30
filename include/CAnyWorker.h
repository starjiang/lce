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
        void setSession(const StSession &stSession)
        {
            this->stSession = stSession;
        }

        StSession &getSession()
        {
            return stSession;
        }

        const StSession &getSession() const
        {
            return stSession;
        }

        CAnyPackage &getReader()
        {
            return oAnyReader;
        }

    private:
        StSession stSession;
        CAnyPackage oAnyReader;
    };

    class CAnyResponse
    {
    public:
        CAnyResponse() : bClose(false)
        {
        }

        void setSession(const StSession &stSession)
        {
            this->stSession = stSession;
        }

        CAnyPackage &getWriter()
        {
            return oAnyWriter;
        }

        StSession &getSession()
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
        StSession stSession;
        CAnyPackage oAnyWriter;
        bool bClose;
    };

    class CAnyWorker
    {
    public:
        CAnyWorker() {}
        virtual ~CAnyWorker() {}

    public:
        virtual void onRequest(CAnyRequest &oRequest, CAnyResponse &oResponse) = 0;
    };

};

#endif // CWORKER_H
