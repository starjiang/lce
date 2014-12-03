#ifndef __NCE_CPROCESSOR_H
#define __NCE_CPROCESSOR_H

#include <stdexcept>

namespace lce
{

struct SSession;

class CProcessor
{
public:
	virtual void onRead(SSession &stSession,const char * pszData, const int iSize){
		throw std::runtime_error("not implement CProcessor onRead");
	}
	virtual void onClose(SSession &stSession){
		throw std::runtime_error("not implement CProcessor onColse");
	}
	virtual void onConnect(SSession &stSession,bool bOk,void *pData){
		throw std::runtime_error("not implement CProcessor onConnect");
	}
	virtual void onError(SSession &stSession,const char * szErrMsg,int iError){
		throw std::runtime_error("not implement CProcessor onError");
	}
	virtual void onTimer(int iTimeId,void *pData){
		throw std::runtime_error("not implement CProcessor onTimer");
	}
	virtual void onMessage(int iMsgType,void *pData){
		throw std::runtime_error("not implement CProcessor onMessage");
	}
	virtual void onSignal(int iSignal){
		throw std::runtime_error("not implement CProcessor onSignal");
	}

	virtual ~CProcessor(){}
};

};

#endif
