#ifndef __LCE_BQUEUE_H__
#define __LCE_BQUEUE_H__

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdexcept>
#include <iostream>
#include <assert.h>
#include <iostream>
#include <string.h>


using namespace std;

namespace lce
{

class CBufferQueue
{
public:
    CBufferQueue()
    {
        _header = NULL;
        _data = NULL;
    }
    ~CBufferQueue()
    {
        _header = NULL;
        _data = NULL;
    }

    void attach(char* pBuf, unsigned long iBufSize) throw (std::runtime_error)
    {
        if(iBufSize <= sizeof(Header)+ulMarkLen+ulReserveLen)
        {
            throw std::runtime_error("bufferqueue::attach fail:iBufSize is too small");
        }

        _header = (Header *)pBuf;
        _data = pBuf+sizeof(Header);
        if(_header->iBufSize != iBufSize - sizeof(Header))
            throw std::runtime_error("bufferqueue::attach fail: iBufSize != iBufSize - sizeof(Header);");
        if(_header->iulReserveLen != ulReserveLen)
            throw std::runtime_error("bufferqueue::attach fail: iulReserveLen != ulReserveLen");
        if(_header->iBegin >= _header->iBufSize)
            throw std::runtime_error("bufferqueue::attach fail: iBegin > iBufSize - sizeof(Header);");
        if(_header->iEnd > iBufSize - sizeof(Header))
            throw std::runtime_error("bufferqueue::attach fail: iEnd > iBufSize - sizeof(Header);");
    }

    void create(char* pBuf, unsigned long iBufSize) throw (std::runtime_error)
    {
        if(iBufSize <= sizeof(Header)+ulMarkLen+ulReserveLen)
        {
            throw std::runtime_error("bufferqueue::create fail:iBufSize is too small");
        }

        _header = (Header *)pBuf;
        _data = pBuf+sizeof(Header);
        _header->iBufSize = iBufSize - sizeof(Header);
        _header->iulReserveLen = ulReserveLen;
        _header->iBegin = 0;
        _header->iEnd = 0;
        _header->iNum = 0;
    }

    bool pop(char *buffer,unsigned long & buffersize) throw(runtime_error)
    {
        unsigned long iEnd=_header->iEnd;
        unsigned tmp_num;
        if(_header->iBegin == iEnd)
        {
            _header->iNum = 0;
            return false;
        }
        else if(_header->iBegin<iEnd)
        {
            assert(_header->iBegin+ulMarkLen < iEnd);
            unsigned long len = getLen(_data+_header->iBegin);
            assert(_header->iBegin+ulMarkLen+len <= iEnd);
            if(len > buffersize)
            {
                _header->iBegin += len+ulMarkLen;
                tmp_num = _header->iNum;
                if(tmp_num > 0)
                    _header->iNum = tmp_num-1;
                throw runtime_error("bufferqueue::pop data is too long to store in the buffer");
            }
            buffersize = len;
            memcpy(buffer,_data+_header->iBegin+ulMarkLen,len);
            _header->iBegin += len+ulMarkLen;
            tmp_num = _header->iNum;
            if(tmp_num > 0)
                _header->iNum = tmp_num-1;
        }
        else
        {
            assert(iEnd+ulReserveLen <= _header->iBegin);
            unsigned long len = 0;
            unsigned long new_begin = 0;
            char *data_from = NULL;
            char *data_to = NULL;
            assert(_header->iBegin < _header->iBufSize);
            if(_header->iBegin+ulMarkLen > _header->iBufSize)
            {
                char tmp[16];
                memcpy(tmp,_data+_header->iBegin,_header->iBufSize-_header->iBegin);
                memcpy(tmp+_header->iBufSize-_header->iBegin,_data,_header->iBegin+ulMarkLen-_header->iBufSize);
                len = getLen(tmp);
                data_from = _data+(_header->iBegin+ulMarkLen-_header->iBufSize); //
                new_begin = _header->iBegin+ulMarkLen-_header->iBufSize+len;
                assert(new_begin <= iEnd);
            }
            else
            {
                len = getLen(_data+_header->iBegin);
                data_from = _data+_header->iBegin+ulMarkLen;
                if(data_from == _data+_header->iBufSize) data_from = _data;
                if(_header->iBegin+ulMarkLen+len < _header->iBufSize)
                {
                    new_begin = _header->iBegin+ulMarkLen+len;
                }
                else     
                {
                    new_begin = _header->iBegin+ulMarkLen+len-_header->iBufSize;
                    assert(new_begin <= iEnd);
                }
            }
            data_to = _data+new_begin;

            if(len > buffersize)
            {
                _header->iBegin = new_begin;
                tmp_num = _header->iNum;
                if(tmp_num > 0)
                    _header->iNum = tmp_num-1;
                throw runtime_error("bufferqueue::pop data is too long to store in the buffer");
            }
            buffersize = len;
            if(data_to > data_from)
            {
                assert(data_to - data_from == (long)len);
                memcpy(buffer,data_from,len);
            }
            else
            {
                memcpy(buffer,data_from,_data+_header->iBufSize-data_from);
                memcpy(buffer+(_data+_header->iBufSize-data_from),_data,data_to-_data);
                assert(_header->iBufSize-(data_from-data_to)== len);
            }
            _header->iBegin = new_begin;
            tmp_num = _header->iNum;
            if(tmp_num > 0)
                _header->iNum = tmp_num-1;
        }

        return true;
    }

    void push(const char *buffer,unsigned long len) throw(runtime_error)
    {
        if(len == 0) return;
        unsigned long iBegin = _header->iBegin;
        if(_header->iEnd == iBegin)
        {
            _header->iNum = 0;
            if(ulMarkLen+len+ulReserveLen>_header->iBufSize)
                throw runtime_error("bufferqueue::push full");
        }
        else if(_header->iEnd > iBegin)
        {
            assert(iBegin+ulMarkLen < _header->iEnd);
            if(_header->iBufSize - _header->iEnd + iBegin < ulMarkLen+len+ulReserveLen)
                throw runtime_error("bufferqueue::push full");
        }
        else
        {
            assert(_header->iEnd+ulReserveLen <= iBegin);
            if(iBegin - _header->iEnd < ulMarkLen+len+ulReserveLen)
                throw runtime_error("bufferqueue::push full");
        }

        if(_header->iEnd+ulMarkLen > _header->iBufSize)
        {
            char tmp[16];
            setLen(tmp,len);
            memcpy(_data+_header->iEnd,tmp,_header->iBufSize-_header->iEnd);
            memcpy(_data,tmp+_header->iBufSize-_header->iEnd,_header->iEnd+ulMarkLen-_header->iBufSize);
            memcpy(_data+_header->iEnd+ulMarkLen-_header->iBufSize,buffer,len);
            _header->iEnd = len+_header->iEnd+ulMarkLen-_header->iBufSize;
            assert(_header->iEnd+ulReserveLen <= iBegin);
            _header->iNum++;
        }
        else if(_header->iEnd+ulMarkLen+len > _header->iBufSize)
        {
            setLen(_data+_header->iEnd,len);
            memcpy(_data+_header->iEnd+ulMarkLen,buffer,_header->iBufSize-_header->iEnd-ulMarkLen);
            memcpy(_data,buffer+_header->iBufSize-_header->iEnd-ulMarkLen,len-(_header->iBufSize-_header->iEnd-ulMarkLen));
            _header->iEnd = len-(_header->iBufSize-_header->iEnd-ulMarkLen);
            assert(_header->iEnd+ulReserveLen <= iBegin);
            _header->iNum++;
        }
        else
        {
            setLen(_data+_header->iEnd,len);
            memcpy(_data+_header->iEnd+ulMarkLen,buffer,len);
            _header->iEnd = (_header->iEnd+ulMarkLen+len)%_header->iBufSize;
            _header->iNum++;
        }
    }

    bool pop(char *buffer1,unsigned long &buffersize1,char *buffer2,unsigned long &buffersize2) throw(runtime_error)
    {
        unsigned long buffersize = buffersize1+buffersize2;
        unsigned long iEnd=_header->iEnd;
        unsigned long tmp_num;
        if(_header->iBegin == iEnd)
        {
            _header->iNum = 0;
            return false;
        }
        else if(_header->iBegin < iEnd)
        {
            assert(_header->iBegin+ulMarkLen < iEnd);
            unsigned long len = getLen(_data+_header->iBegin);
            assert(_header->iBegin+ulMarkLen+len <= iEnd);
            if(len > buffersize)
            {
                _header->iBegin += len+ulMarkLen;
                tmp_num = _header->iNum;
                if(tmp_num > 0)
                    _header->iNum = tmp_num-1;
                throw runtime_error("bufferqueue::pop data is too long to store in the buffer");
            }
            if(buffersize1 > len)
            {
                buffersize1 = len;
                buffersize2 = 0;
                memcpy(buffer1,_data+_header->iBegin+ulMarkLen,len);
            }
            else
            {
                buffersize2 = len-buffersize1;
                memcpy(buffer1,_data+_header->iBegin+ulMarkLen,buffersize1);
                memcpy(buffer2,_data+_header->iBegin+ulMarkLen+buffersize1,buffersize2);
            }
            _header->iBegin += len+ulMarkLen;
            tmp_num = _header->iNum;
            if(tmp_num > 0)
                _header->iNum = tmp_num-1;
        }
        else
        {
            assert(iEnd+ulReserveLen <= _header->iBegin);
            unsigned long len = 0;
            unsigned long new_begin = 0;
            char *data_from = NULL;
            char *data_to = NULL;
            assert(_header->iBegin < _header->iBufSize);
            if(_header->iBegin+ulMarkLen > _header->iBufSize)
            {
                char tmp[16];
                memcpy(tmp,_data+_header->iBegin,_header->iBufSize-_header->iBegin);
                memcpy(tmp+_header->iBufSize-_header->iBegin,_data,_header->iBegin+ulMarkLen-_header->iBufSize);
                len = getLen(tmp);
                data_from = _data+(_header->iBegin+ulMarkLen-_header->iBufSize); //
                new_begin = (_header->iBegin+ulMarkLen-_header->iBufSize)+len;
                assert(new_begin <= iEnd);
            }
            else
            {
                len = getLen(_data+_header->iBegin);
                data_from = _data+_header->iBegin+ulMarkLen;
                if(data_from == _data+_header->iBufSize) data_from = _data;
                if(_header->iBegin+ulMarkLen+len < _header->iBufSize)
                {
                    new_begin = _header->iBegin+ulMarkLen+len;
                }
                else     // ���ݱ��ֶ�
                {
                    new_begin = _header->iBegin+ulMarkLen+len-_header->iBufSize;
                    assert(new_begin <= iEnd);
                }
            }
            data_to = _data+new_begin;

            if(len > buffersize)
            {
                _header->iBegin = new_begin;
                tmp_num = _header->iNum;
                if(tmp_num > 0)
                    _header->iNum = tmp_num-1;
                throw runtime_error("bufferqueue::pop data is too long to store in the buffer");
            }
            buffersize = len;
            if(data_to > data_from)
            {
                assert(data_to - data_from == (long)len);
                if(buffersize1 > len)
                {
                    buffersize1 = len;
                    buffersize2 = 0;
                    memcpy(buffer1,data_from,len);
                }
                else
                {
                    buffersize2 = len-buffersize1;
                    memcpy(buffer1,data_from,buffersize1);
                    memcpy(buffer2,data_from+buffersize1,buffersize2);
                }
            }
            else
            {
                assert(_header->iBufSize-(data_from-data_to)== len);
                if(buffersize1>len)
                {
                    buffersize1 = len;
                    buffersize2 = 0;
                    memcpy(buffer1,data_from,_data+_header->iBufSize-data_from);
                    memcpy(buffer1+(_data+_header->iBufSize-data_from),_data,data_to-_data);
                }
                else if(buffersize1>_data-data_from+_header->iBufSize)     
                {
                    buffersize2 = len-buffersize1;
                    memcpy(buffer1,data_from,_data+_header->iBufSize-data_from);
                    memcpy(buffer1+(_data+_header->iBufSize-data_from),_data,buffersize1-(_data+_header->iBufSize-data_from));
                    memcpy(buffer2,data_from+buffersize1-_header->iBufSize,buffersize2);
                }
                else     
                {
                    buffersize2 = len-buffersize1;
                    memcpy(buffer1,data_from,buffersize1);
                    memcpy(buffer2,data_from+buffersize1,_data+_header->iBufSize-data_from-buffersize1);
                    memcpy(buffer2+(_data+_header->iBufSize-data_from-buffersize1),_data,len-(_data-data_from+_header->iBufSize));
                }
            }
            _header->iBegin = new_begin;
            tmp_num = _header->iNum;
            if(tmp_num > 0)
                _header->iNum = tmp_num-1;
        }

        return true;
    }

    void push(const char *buffer1,unsigned len1,const char *buffer2,unsigned len2) throw(runtime_error)
    {
        unsigned long len = len1+len2;
        if(len == 0) return;
        unsigned long iBegin = _header->iBegin;
        if(_header->iEnd == iBegin)
        {
            _header->iNum = 0;
            if(ulMarkLen+len+ulReserveLen>_header->iBufSize)
                throw runtime_error("bufferqueue::push full");
        }
        else if(_header->iEnd > iBegin)
        {
            assert(iBegin+ulMarkLen < _header->iEnd);
            if(_header->iBufSize - _header->iEnd + iBegin < ulMarkLen+len+ulReserveLen)
                throw runtime_error("bufferqueue::push full");
        }
        else
        {
            assert(_header->iEnd+ulReserveLen <= iBegin);
            if(iBegin - _header->iEnd < ulMarkLen+len+ulReserveLen)
                throw runtime_error("bufferqueue::push full");
        }

        if(_header->iEnd+ulMarkLen > _header->iBufSize)
        {
            char tmp[16];
            setLen(tmp,len);
            memcpy(_data+_header->iEnd,tmp,_header->iBufSize-_header->iEnd);
            memcpy(_data,tmp+_header->iBufSize-_header->iEnd,_header->iEnd+ulMarkLen-_header->iBufSize);
            memcpy(_data+_header->iEnd+ulMarkLen-_header->iBufSize,buffer1,len1);
            memcpy(_data+_header->iEnd+ulMarkLen-_header->iBufSize+len1,buffer2,len2);
            _header->iEnd = len+_header->iEnd+ulMarkLen-_header->iBufSize;
            assert(_header->iEnd+ulReserveLen <= iBegin);
            _header->iNum++;
        }
        else if(_header->iEnd+ulMarkLen+len > _header->iBufSize)
        {
            setLen(_data+_header->iEnd,len);
            if(_header->iEnd+ulMarkLen+len1>_header->iBufSize)   
            {
                memcpy(_data+_header->iEnd+ulMarkLen,buffer1,_header->iBufSize-_header->iEnd-ulMarkLen);
                memcpy(_data,buffer1+_header->iBufSize-_header->iEnd-ulMarkLen,len1-(_header->iBufSize-_header->iEnd-ulMarkLen));
                memcpy(_data+len1-(_header->iBufSize-_header->iEnd-ulMarkLen),buffer2,len2);
            }
            else    
            {
                memcpy(_data+_header->iEnd+ulMarkLen,buffer1,len1);
                memcpy(_data+_header->iEnd+ulMarkLen+len1,buffer2,_header->iBufSize-_header->iEnd-ulMarkLen-len1);
                memcpy(_data,buffer2+_header->iBufSize-_header->iEnd-ulMarkLen-len1,len2-(_header->iBufSize-_header->iEnd-ulMarkLen-len1));
            }
            _header->iEnd = len-(_header->iBufSize-_header->iEnd-ulMarkLen);
            assert(_header->iEnd+ulReserveLen <= iBegin);
            _header->iNum++;
        }
        else
        {
            setLen(_data+_header->iEnd,len);
            memcpy(_data+_header->iEnd+ulMarkLen,buffer1,len1);
            memcpy(_data+_header->iEnd+ulMarkLen+len1,buffer2,len2);
            _header->iEnd = (_header->iEnd+ulMarkLen+len)%_header->iBufSize;
            _header->iNum++;
        }
    }
    bool empty() const
    {
        unsigned long iEnd=_header->iEnd;
        return _header->iBegin == iEnd;
    }

    bool full(unsigned long len) const
    {
        unsigned long iBegin = _header->iBegin;
        if(len==0) return false;

        if(_header->iEnd == iBegin)
        {
            if(len+ulMarkLen+ulReserveLen > _header->iBufSize) return true;
            return false;
        }
        else if(_header->iEnd > iBegin)
        {
            assert(iBegin+ulMarkLen < _header->iEnd);
            return _header->iBufSize - _header->iEnd + iBegin < ulMarkLen+len+ulReserveLen;
        }
        assert(_header->iEnd+ulReserveLen <= iBegin);
        return (iBegin - _header->iEnd < ulMarkLen+len+ulReserveLen);
    }
    unsigned long size() const
    {
        if (empty())
        {
            _header->iNum = 0;
        }
        return _header->iNum;
    }

private:
    unsigned long getLen(char *buf)
    {
        unsigned long u;
        memcpy((void *)&u,buf,ulMarkLen);
        return u;
    }
    void setLen(char *buf,unsigned long u)
    {
        memcpy(buf,(void *)&u,ulMarkLen);
    }

private:
    const static unsigned long ulReserveLen = 8;
    const static unsigned long ulMarkLen = sizeof(unsigned long);
    struct Header
    {
        unsigned long iBufSize;
        unsigned long iulReserveLen; // must be 8
        unsigned long iBegin;
        unsigned long iEnd;
        unsigned long iNum;		
    };

    Header *_header;
    char *_data;
};

}
#endif
