#ifndef __LCE_CPACKAGEFILTER_H
#define __LCE_CPACKAGEFILTER_H

namespace lce
{
class CPackageFilter
{

public:
    // return value: -2 invalid pkt, -1 pkt haven't received whole pkt, 0 normal
    virtual int isWholePkg(const char* pszData, const int iDataSize, int& iRealPkgLen, int& iPkgLen) = 0;
    virtual const char* getRealPkgData(const char* pszData, const int iDataSize) = 0;
    virtual ~CPackageFilter(){}
};


}

#endif
