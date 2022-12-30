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

        CHttpReader &getReader()
        {
            return oHttpReader;
        }

    private:
        StSession stSession;
        CHttpReader oHttpReader;
    };

    class CHttpResponse
    {
    public:
        CHttpResponse() : bClose(false)
        {
        }

        void setSession(const StSession &stSession)
        {
            this->stSession = stSession;
        }

        CHttpWriter &getWriter()
        {
            return oHttpWriter;
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
        CHttpWriter oHttpWriter;
        bool bClose;
    };

    class CHttpWorker
    {
    public:
        CHttpWorker() {}
        virtual ~CHttpWorker() {}

    public:
        virtual void onRequest(CHttpRequest &oRequest, CHttpResponse &oResponse) = 0;
    };

};

#endif // CWORKER_H
