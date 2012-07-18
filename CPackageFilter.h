#ifndef __NCE_CPACKAGEFILTER_H
#define __NCE_CPACKAGEFILTER_H

namespace lce
{
class CPackageFilter
{

public:
    // return value: -2:非法包; -1:不完整包; 0:完整包
    virtual int isWholePkg(const char* pszData, const int iDataSize, int& iRealPkgLen, int& iPkgLen) = 0;
    virtual const char* getRealPkgData(const char* pszData, const int iDataSize) = 0;
};


}

#endif
