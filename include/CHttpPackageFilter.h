#ifndef __NCE_CHTTPPACKAGE_H
#define __NCE_CHTTPPACKAGE_H

namespace lce
{
class CHttpPackageFilter :public CPackageFilter
{
	enum
	{
		MAX_PKG_SIZE=(unsigned int)-1,
	};

public:
    // return value: -2:非法包; -1:不完整包; 0:完整包
    inline int isWholePkg(const char* pszData, const int iDataSize, int& iRealPkgLen, int& iPkgLen)
    {
       int iRe = -1;
		const char* pFirstLine = ::strstr(pszData, "\r\n");
		if ( NULL != pFirstLine )
		{
			const char* pEnd = ::strstr(pszData, "\r\n\r\n")+sizeof("\r\n\r\n")-1;
			if ( NULL != pEnd )
			{

				bool bRecvData = false;
				const char* pHTTP = ::strstr(pszData, "HTTP");
				if ( pHTTP == pszData )
				{
					bRecvData = true;
				}

				int iHeadLen = (int)(pEnd-pszData);
				int iHTMLLen = iHeadLen;

				if ( bRecvData )	//接收请求返回的数据
				{
					const char* pContentLenPos = ::strstr(pszData, "Content-Length: ");
					if ( NULL != pContentLenPos )
					{
						if ( pEnd > pContentLenPos )
						{
							const char* pContentLenEndPos = strstr(pContentLenPos, "\r\n");
							if ( NULL != pContentLenEndPos && pContentLenPos < pEnd )
							{
								//bug modified by alex 2008-11-15
								//int iContentLen = atoi(pContentLenPos);+strlen("Content-Length: ")
								int iContentLen = atoi(pContentLenPos+sizeof("Content-Length: ")-1);
								iHTMLLen += iContentLen;
							}
						}

						if ( iHTMLLen <= iDataSize && iHTMLLen > iHeadLen )
						{
							//alex 2008-11-15
							//iRealPkgLen = iPkgLen = iDataSize;
							iRealPkgLen = iPkgLen = iHTMLLen;
							//printf("接收请求返回的数据 iDataSize=%d, iHTMLLen=%d\n", iDataSize, iHTMLLen);
							iRe = 0;
						}
					}
					if ( iHTMLLen <= iDataSize && iHTMLLen > 0 )
					{
						//happy 2009-3-22
						//iRealPkgLen = iPkgLen = iDataSize;
						iRealPkgLen = iPkgLen = iHTMLLen;
						//printf("接入http请求 iDataSize=%d\n", iDataSize);
						//printf("*****http_pkg=%s*****\n", pszData);
						iRe = 0;
					}
				}
				else	//接入http请求
				{
					const char* pContentLenPos = ::strstr(pszData, "Content-Length: ");
					if ( NULL != pContentLenPos )
					{
						if ( pEnd > pContentLenPos )
						{
							const char* pContentLenEndPos = strstr(pContentLenPos, "\r\n");
							if ( NULL != pContentLenEndPos && pContentLenPos < pEnd )
							{
								//bug modified by alex 2008-11-15
								//int iContentLen = atoi(pContentLenPos);
								int iContentLen = atoi(pContentLenPos+sizeof("Content-Length: ")-1);
								iHTMLLen += iContentLen;
							}
						}
					}
					if ( iHTMLLen <= iDataSize && iHTMLLen > 0 )
					{
						//alex 2008-11-15
						//iRealPkgLen = iPkgLen = iDataSize;
						iRealPkgLen = iPkgLen = iHTMLLen;
						//printf("接入http请求 iDataSize=%d\n", iDataSize);
						iRe = 0;
					}
				}
			}
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
