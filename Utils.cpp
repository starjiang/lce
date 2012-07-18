#include "Utils.h"

namespace lce
{

int listen(int iFd)
{
    if(::listen(iFd,1024) == -1)
    {
        return -1;
    }
    return 0;
}

int setNBlock(int iFd)
{
    int opts;
    opts=fcntl(iFd,F_GETFL);
    if(opts<0)
    {
        return -1;
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(iFd,F_SETFL,opts)<0)
    {
        return -1;
    }
    return 0;
}

int close(const int iFd)
{
    return ::close(iFd);
}

int setNCloseWait(const int iFd)
{
    linger sLinger;
    sLinger.l_onoff = 0;  // (在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
    sLinger.l_linger = 0; // (容许逗留的时间为0秒)
    if(setsockopt(iFd, SOL_SOCKET,	SO_LINGER,	(char*)&sLinger, sizeof(linger)) != 0)
    {
        return -1;
    }
    // 如果服务器终止后,服务器可以第二次快速启动而不用等待一段时间
    int nREUSEADDR = 1;
    if(setsockopt(iFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&nREUSEADDR, sizeof(int)) != 0)
    {
        return -1;
    }
    return 0;
}

int send(int iFd, const char *buf, int count)
{
    return ::send(iFd, buf, count, 0);
}

int createTcpSock()
{
    int iFd;
    if((iFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        return -1;
    }
    return iFd;
}

int createUdpSock()
{
    int iFd;
    if((iFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        return -1;
    }
    return iFd;
}

int bind(int iFd,const std::string &sHost,uint16_t wPort)
{
    struct sockaddr_in my_addr;
    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(wPort);
    my_addr.sin_addr.s_addr = inet_addr(sHost.c_str());
    memset(&(my_addr.sin_zero),0,8);
    if(::bind(iFd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))==-1)
    {
        return -1;
    }
    return 0;
}

int connect(int iFd,const std::string &sHost,uint16_t wPort)
{
    struct sockaddr_in remote_addr;

    remote_addr.sin_family=AF_INET;
    remote_addr.sin_port=htons(wPort);
    remote_addr.sin_addr.s_addr = inet_addr(sHost.c_str());
    memset(&(remote_addr.sin_zero),0,8);

    return ::connect(iFd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr));

}


int sendto(int iFd,const char *buf,int count,const std::string &sIp,uint16_t wPort)
{
    struct sockaddr_in remote_addr;

    remote_addr.sin_family=AF_INET;
    remote_addr.sin_port=htons(wPort);
    remote_addr.sin_addr.s_addr = inet_addr(sIp.c_str());
    memset(&(remote_addr.sin_zero),0,8);
    return ::sendto(iFd,buf,count,0,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr));
}


//去掉字符串2头的空格
std::string& trimString(std::string& sSource)
{
    if (sSource.size()==0)
        return sSource;

    std::string sDest = "";

    size_t i = 0;
    for(; i < sSource.size() &&
            (sSource[i] == ' '  ||
             sSource[i] == '\r'  ||
             sSource[i] == '\n'  ||
             sSource[i] == '\t'); i++);

    size_t j = sSource.size() - 1;
    if(j > 0)
    {
        for(; j > 0 &&
                (sSource[j] == ' '   ||
                 sSource[j] == '\r'  ||
                 sSource[j] == '\n'  ||
                 sSource[j] == '\t')
                ; j--);
    }

    if(j >= i)
    {
        sDest = sSource.substr(i, j-i+1);
        sSource = sDest;
    }
    else
        sSource = "";
    return sSource;
}

void ignorePipe(void)
{
    struct sigaction sig;
    sig.sa_handler = SIG_IGN;
    sig.sa_flags = 0;
    sigemptyset(&sig.sa_mask);
    //防止向关闭的fd写数据，产生sigpipe信号使程序退出
    sigaction(SIGPIPE,&sig,NULL);
}

void initDaemon()
{
    pid_t pid;
    if ((pid = fork()) != 0)
        exit(0);
    setsid();
    signal(SIGINT,  SIG_IGN);
    signal(SIGHUP,  SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGIO,   SIG_IGN);
    ignorePipe();

    if ((pid = fork()) != 0)
        exit(0);
    umask(0);
}

bool setFileLimit(const uint32_t dwLimit)
{
    bool bOK = false;
    struct rlimit rlim = {0};
    if(getrlimit((__rlimit_resource_t)RLIMIT_OFILE, &rlim) == 0)
    {
        //		int cur = rlim.rlim_cur;
        if(rlim.rlim_cur < dwLimit)
        {
            rlim.rlim_cur = dwLimit;
            rlim.rlim_max = dwLimit;
            if(setrlimit((__rlimit_resource_t)RLIMIT_OFILE, &rlim) == 0)
            {
                bOK = true;
            }
        }
        else
        {
            bOK = true;
        }
    }
    return bOK;
}

bool setCoreLimit(const uint32_t dwLimit)
{
    bool bOK = false;
    struct rlimit rlim = {0};
    if(getrlimit((__rlimit_resource_t)RLIMIT_CORE, &rlim) == 0)
    {
        //		int cur = rlim.rlim_cur;
        if(rlim.rlim_cur < dwLimit)
        {
            rlim.rlim_cur = dwLimit;
            rlim.rlim_max = dwLimit;
            if(setrlimit((__rlimit_resource_t)RLIMIT_CORE, &rlim) == 0)
            {
                bOK = true;
            }
        }
        else
        {
            bOK = true;
        }
    }
    return bOK;
}

const static char *days[]= {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
const static char *months[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

std::string getGMTDate(const time_t& cur)
{
    char buf[256];
    struct tm  *gt = gmtime(&cur);
    snprintf(buf, sizeof(buf),"%s, %02d %s %04d %02d:%02d:%02d GMT",
             days[gt->tm_wday],
             gt->tm_mday,
             months[gt->tm_mon],
             gt->tm_year + 1900,
             gt->tm_hour,
             gt->tm_min,
             gt->tm_sec);
    return buf;
}

time_t gmt2Time(const char *str)
{
#define BAD	0
#define GC	(c=*tstr++)
    const unsigned char *tstr = reinterpret_cast<const unsigned char*>(str);
    char c;
    int year = -1, month, day = -1, sec;

    while(GC != ' ')
        if(!c) return BAD;
    if(isdigit(GC))
    {
        day = c - '0';
        if(isdigit(GC))
        {
            day = day * 10 + c - '0';
            GC;
        }
        if(c!=' ' && c!='-') return BAD;
        GC;
    }
    switch(c)
    {
    case 'A': /*Apr,Aug*/
        GC;
        if(c=='p')
        {
            if(GC != 'r') return BAD;
            month = 2;
            break;
        }
        else if(c=='u')
        {
            if(GC != 'g') return BAD;
            month = 6;
            break;
        }
        return BAD;
    case 'D': /*Dec*/
        if(GC != 'e') return BAD;
        if(GC != 'c') return BAD;
        month = 10;
        break;
    case 'F': /*Feb*/
        if(GC != 'e') return BAD;
        if(GC != 'b') return BAD;
        month = 12;
        break;
    case 'J': /*Jan,Jun,Jul*/
        GC;
        if(c=='a')
        {
            if(GC != 'n') return BAD;
            month = 11;
            break;
        }
        else if(c != 'u')
        {
            return BAD;
        }
        GC;
        if(c == 'n')
        {
            month = 4;
            break;
        }
        else if(c=='l')
        {
            month = 5;
            break;
        }
        return BAD;
    case 'M': /*Mar,May*/
        if(GC != 'a') return BAD;
        GC;
        if(c=='r')
        {
            month = 1;
            break;
        }
        else if(c=='y')
        {
            month = 3;
            break;
        }
        return BAD;
    case 'N': /*Nov*/
        if(GC != 'o') return BAD;
        if(GC != 'v') return BAD;
        month = 9;
        break;
    case 'O': /*Oct*/
        if(GC != 'c') return BAD;
        if(GC != 't') return BAD;
        month = 8;
        break;
    case 'S': /*Sep*/
        if(GC != 'e') return BAD;
        if(GC != 'p') return BAD;
        month = 7;
        break;
    default:
        return BAD;
    }
    GC;
    if(c!=' ' && c!='-') return BAD;
    GC;
    if(c==' ') GC;
    if(day==-1)
    {
        day = c - '0';
        if(isdigit(GC))
        {
            day = day * 10 + c - '0';
            GC;
        }
    }
    else
    {
        year = c - '0';
        if(isdigit(GC))
        {
            year = year * 10 + c - '0';
            if(isdigit(GC))
            {
                year = year * 10 + c - '0';
                if(isdigit(GC))
                {
                    year = year * 10 + c - '0';
                    GC;
                }
            }
        }
    }
    if(c!=' ') return BAD;
    GC;
    if(!isdigit(c)) return BAD;
    sec = (c-'0') * 36000;
    GC;
    if(!isdigit(c)) return BAD;
    sec += (c-'0') * 3600;
    GC;
    if(c!=':') return BAD;
    GC;
    if(!isdigit(c)) return BAD;
    sec += (c-'0') * 600;
    GC;
    if(!isdigit(c)) return BAD;
    sec += (c-'0') * 60;
    GC;
    if(c!=':') return BAD;
    GC;
    if(!isdigit(c)) return BAD;
    sec += (c-'0') * 10;
    GC;
    if(!isdigit(c)) return BAD;
    sec += c-'0';
    GC;
    if(c!=' ') return BAD;
    if(year==-1)
    {
        if(!isdigit(GC)) return BAD;
        year = (c-'0') * 1000;
        if(!isdigit(GC)) return BAD;
        year += (c-'0') * 100;
        if(!isdigit(GC)) return BAD;
        year += (c-'0') * 10;
        if(!isdigit(GC)) return BAD;
        year += c-'0';
    }
    if(year < 70)
        year += 32;
    else if(year < 100)
        year -= 68;
    else if(year < 1970)
        return BAD;
    else if(year < 2040)
        year -= 1968;
    else
        return BAD;
    if(month > 10) year--;
    year = year * 365 + year/4 + month*520/17 + day;
    return year * 86400 + sec - 60652800;
}

std::string formUrlEncode(const std::string &sSrc)
{
    std::string sResult;
    for(std::string::const_iterator iter = sSrc.begin(); iter != sSrc.end(); ++iter)
    {
        switch(*iter)
        {
        case ' ':
            sResult.append(1, '+');
            break;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
        case '_':
        case '.':
            sResult.append(1, *iter);
            break;
        default:
            sResult.append(1, '%');
            sResult.append(charToHex(*iter));
            break;
        }
    }

    return sResult;
}

std::string formUrlDecode(const std::string &sSrc)
{
    std::string sResult = "";
    int iLen = sSrc.length();

    for (int i = 0; i < iLen; i++)
    {
        int j = i ;
        char ch = sSrc.at(j);
        if(ch =='+')
        {
            sResult+=' ';
        }
        else if (ch == '%')
        {
            char szTmpStr[] = "0x0__";
            int iChNum;
            szTmpStr[3] = sSrc.at(j+1);
            szTmpStr[4] = sSrc.at(j+2);
            iChNum = strtol(szTmpStr, NULL, 16);
            sResult += iChNum;
            i += 2;
        }
        else
        {
            sResult += ch;
        }
    }

    return sResult;

}

std::string charToHex(char c)
{
    std::string sResult;
    char first, second;

    first = (c & 0xF0) / 16;
    first += first > 9 ? 'A' - 10 : '0';
    second = c & 0x0F;
    second += second > 9 ? 'A' - 10 : '0';

    sResult.append(1, first);
    sResult.append(1, second);

    return sResult;
}

std::string textEncodeJSON(const std::string& sSrc, const bool bUtf8)
{
	std::string sDesc;
	for (std::string::size_type i=0; i<sSrc.size(); i++)
	{
		if ( bUtf8 )
		{
			int iUtf8Size =  NCE_UTF8_LENGTH(sSrc[i]);
			if ( iUtf8Size == 1 )
			{
				if (sSrc[i] == '\"')
				{
					sDesc += "\\\"";
				}
				else if (sSrc[i] == '\r')
				{
					sDesc += "\\r";
				}
				else if (sSrc[i] == '\n')
				{
					sDesc += "\\n";
				}
				else if (sSrc[i] == '\\')
				{
					sDesc += "\\\\";
				}
				else if (sSrc[i] == '/')
				{
					sDesc += "\\/";
				}
				else if (sSrc[i] == '\b')
				{
					sDesc += "\\b";
				}
				else if (sSrc[i] == '\f')
				{
					sDesc += "\\f";
				}
				else if (sSrc[i] == '\t')
				{
					sDesc += "\\t";
				}
				else
				{
					sDesc += sSrc[i];
				}
			}
			else
			{
				if(iUtf8Size > 1)
				{
					if ( iUtf8Size+i<=sSrc.size() )
					{
						sDesc.append(sSrc.data()+i, iUtf8Size);
						i += iUtf8Size-1;
					}
					else
					{
						i += iUtf8Size-1;
					}
				}
			}
		}
		else
		{
			if (sSrc[i] == '\"')
			{
				sDesc += "\\\"";
			}
			else if (sSrc[i] == '\r')
			{
				sDesc += "\\r";
			}
			else if (sSrc[i] == '\n')
			{
				sDesc += "\\n";
			}
			else if (sSrc[i] == '\\')
			{
				sDesc += "\\\\";
			}
			else if (sSrc[i] == '/')
			{
				sDesc += "\\/";
			}
			else if (sSrc[i] == '\b')
			{
				sDesc += "\\b";
			}
			else if (sSrc[i] == '\f')
			{
				sDesc += "\\f";
			}
			else if (sSrc[i] == '\t')
			{
				sDesc += "\\t";
			}
			else if( (unsigned char)sSrc[i] >= 0x81 && (unsigned char)sSrc[i]<= 0xfe  )
			{
				if ( i+1 != sSrc.size() )
				{
					if ( NCE_IS_GBK_CHAR(sSrc[i], sSrc[i+1]) )
					{
						sDesc += sSrc[i] ;
						sDesc += sSrc[++i] ;
					}
				}
				else
				{
					sDesc += sSrc[i] ;
					sDesc += " ";
				}
			}
			else
			{
				sDesc += sSrc[i] ;
			}
		}
	}
	return sDesc;
}


};

