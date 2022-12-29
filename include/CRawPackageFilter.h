#ifndef __LCE_CRAWPACKAGE_H
#define __LCE_CRAWPACKAGE_H
namespace lce
{
class CRawPackageFilter :public CPackageFilter
{

public:
    // return value: -2 invalid pkt, -1 pkt haven't received whole pkt, 0 normal
    inline int isWholePkg(const char* pszData, const int iDataSize, int& iRealPkgLen, int& iPkgLen)
    {
        int iRe = -2;
		if (iDataSize <= 0)
		{
			return -1;
		}
		else
		{
			iRealPkgLen = iPkgLen = iDataSize;
			iRe = 0;
		}

		return iRe;
    }

    const char* getRealPkgData(const char* pszData, const int iDataSize)
    {
        return pszData;
    }


};

};

#endif