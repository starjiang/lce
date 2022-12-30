#ifndef __LCE_CPROCESSOR_H
#define __LCE_CPROCESSOR_H

#include <stdexcept>

namespace lce
{

	struct StSession;

	class CProcessor
	{
	public:
		virtual void onRead(StSession &stSession, const char *pszData, const int iSize)
		{
			throw std::runtime_error("not implement CProcessor onRead");
		}
		virtual void onClose(StSession &stSession)
		{
			throw std::runtime_error("not implement CProcessor onColse");
		}
		virtual void onConnect(StSession &stSession, bool bOk, void *pData)
		{
			throw std::runtime_error("not implement CProcessor onConnect");
		}
		virtual void onError(StSession &stSession, const char *szErrMsg, int iError)
		{
			throw std::runtime_error("not implement CProcessor onError");
		}
		virtual void onTimer(int iTimeId, void *pData)
		{
			throw std::runtime_error("not implement CProcessor onTimer");
		}
		virtual void onMessage(int iMsgType, void *pData)
		{
			throw std::runtime_error("not implement CProcessor onMessage");
		}
		virtual void onSignal(int iSignal)
		{
			throw std::runtime_error("not implement CProcessor onSignal");
		}

		virtual ~CProcessor() {}
	};

};

#endif
