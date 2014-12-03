#ifndef __NCE_CH2T3_H
#define __NCE_CH2T3_H

namespace lce
{
class CH2T3PackageFilter :public CPackageFilter
{

    enum
    {
		PKG_HEAD_MARK=0x02,
		PKG_TAIL_MARK=0x03,
	};
	enum
	{
		MAX_PKG_SIZE=(unsigned int)0xffffffff,
	};


public:
    // return value: -2:非法包; -1:不完整包; 0:完整包
    inline int isWholePkg(const char* pszData, const int iDataSize, int& iRealPkgLen, int& iPkgLen)
    {
        int iRe = -2;
		if (iDataSize <= 0)
		{
			return -1;
		}

		if (iDataSize <= 5)
		{
			iRe = -1;
			if ( *pszData != PKG_HEAD_MARK )
			{
				iRe = -2;
			}
		}
		else
		{
            if ( *pszData != PKG_HEAD_MARK)
            {
                return -2;
            }

			iPkgLen = ntohl(*((unsigned int*)(pszData+1)));
			if ( iDataSize >= iPkgLen && iPkgLen >= 6  )
			{

				if ( *(pszData+iPkgLen-1) == PKG_TAIL_MARK )
				{
					iRealPkgLen = iPkgLen - 6;
					iRe = 0;
				}
				else
				{
					iRe = -2;
				}
			}
			else
			{
				iRe = -1;
				if ( *pszData != PKG_HEAD_MARK || iPkgLen < 6 )
				{
					iRe = -2;
				}
			}
		}
		return iRe;
    }

    const char* getRealPkgData(const char* pszData, const int iDataSize)
    {
        return pszData+5;
    }


};

};

#endif
