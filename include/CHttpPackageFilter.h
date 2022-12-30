#ifndef __NCE_CHTTPPACKAGE_H
#define __NCE_CHTTPPACKAGE_H

namespace lce
{
	class CHttpPackageFilter : public CPackageFilter
	{
		enum
		{
			MAX_PKG_SIZE = (unsigned int)-1,
		};

	public:
		// return value: -2 invalid pkt, -1 pkt haven't received whole pkt, 0 normal
		inline int isWholePkg(const char *pszData, const int iDataSize, int &iRealPkgLen, int &iPkgLen)
		{
			int iRe = -1;
			const char *pFirstLine = ::strstr(pszData, "\r\n");
			if (NULL != pFirstLine)
			{
				const char *pEnd = ::strstr(pszData, "\r\n\r\n") + sizeof("\r\n\r\n") - 1;
				if (NULL != pEnd)
				{

					bool bRecvData = false;
					const char *pHTTP = ::strstr(pszData, "HTTP");
					if (pHTTP == pszData)
					{
						bRecvData = true;
					}

					int iHeadLen = (int)(pEnd - pszData);
					int iHTMLLen = iHeadLen;

					if (bRecvData)
					{
						const char *pContentLenPos = ::strstr(pszData, "Content-Length: ");
						if (NULL != pContentLenPos)
						{
							if (pEnd > pContentLenPos)
							{
								const char *pContentLenEndPos = strstr(pContentLenPos, "\r\n");
								if (NULL != pContentLenEndPos && pContentLenPos < pEnd)
								{
									int iContentLen = atoi(pContentLenPos + sizeof("Content-Length: ") - 1);
									iHTMLLen += iContentLen;
								}
							}

							if (iHTMLLen <= iDataSize && iHTMLLen > iHeadLen)
							{
								iRealPkgLen = iPkgLen = iHTMLLen;
								iRe = 0;
							}
						}
						if (iHTMLLen <= iDataSize && iHTMLLen > 0)
						{
							iRealPkgLen = iPkgLen = iHTMLLen;
							iRe = 0;
						}
					}
					else
					{
						const char *pContentLenPos = ::strstr(pszData, "Content-Length: ");
						if (NULL != pContentLenPos)
						{
							if (pEnd > pContentLenPos)
							{
								const char *pContentLenEndPos = strstr(pContentLenPos, "\r\n");
								if (NULL != pContentLenEndPos && pContentLenPos < pEnd)
								{
									int iContentLen = atoi(pContentLenPos + sizeof("Content-Length: ") - 1);
									iHTMLLen += iContentLen;
								}
							}
						}
						if (iHTMLLen <= iDataSize && iHTMLLen > 0)
						{
							iRealPkgLen = iPkgLen = iHTMLLen;
							iRe = 0;
						}
					}
				}
			}
			return iRe;
		}

		const char *getRealPkgData(const char *pszData, const int iDataSize)
		{
			return pszData;
		}
	};

};

#endif
