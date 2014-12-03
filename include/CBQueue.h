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

namespace lce {

class CBQueue
{
public:
	CBQueue() {_header = NULL;_data = NULL;}
	~CBQueue() {_header = NULL;_data = NULL;}

	void attach(char* pBuf, unsigned long iBufSize) throw (std::runtime_error)
	{
		if(iBufSize <= sizeof(Header)+MarkLen+ReserveLen) {
			throw std::runtime_error("squeue::attach fail:iBufSize is too small");
		}

		_header = (Header *)pBuf;
		_data = pBuf+sizeof(Header);
		if(_header->iBufSize != iBufSize - sizeof(Header))
			throw std::runtime_error("squeue::attach fail: iBufSize != iBufSize - sizeof(Header);");
		if(_header->iReserveLen != ReserveLen)
			throw std::runtime_error("squeue::attach fail: iReserveLen != ReserveLen");
		if(_header->iBegin >= _header->iBufSize)
			throw std::runtime_error("squeue::attach fail: iBegin > iBufSize - sizeof(Header);");
		if(_header->iEnd > iBufSize - sizeof(Header))
			throw std::runtime_error("squeue::attach fail: iEnd > iBufSize - sizeof(Header);");
	}

	void create(char* pBuf, unsigned long iBufSize) throw (std::runtime_error)
	{
		if(iBufSize <= sizeof(Header)+MarkLen+ReserveLen) {
			throw std::runtime_error("squeue::create fail:iBufSize is too small");
		}

		_header = (Header *)pBuf;
		_data = pBuf+sizeof(Header);
		_header->iBufSize = iBufSize - sizeof(Header);
		_header->iReserveLen = ReserveLen;
		_header->iBegin = 0;
		_header->iEnd = 0;
		_header->iNum = 0;
	}

	// 读端使用
	bool pop(char *buffer,unsigned long & buffersize) throw(runtime_error)
	{
		unsigned long iEnd=_header->iEnd;
		unsigned tmp_num;
		if(_header->iBegin == iEnd) {
            _header->iNum = 0;
			return false;
        }
		else if(_header->iBegin<iEnd) {
			assert(_header->iBegin+MarkLen < iEnd);
			unsigned long len = GetLen(_data+_header->iBegin);
			assert(_header->iBegin+MarkLen+len <= iEnd);
			if(len > buffersize) {
				_header->iBegin += len+MarkLen;
				tmp_num = _header->iNum;
				if(tmp_num > 0) 
					_header->iNum = tmp_num-1;
				throw runtime_error("squeue::pop data is too long to store in the buffer");
			}
			buffersize = len;
			memcpy(buffer,_data+_header->iBegin+MarkLen,len);
			_header->iBegin += len+MarkLen;
			tmp_num = _header->iNum;
			if(tmp_num > 0) 
				_header->iNum = tmp_num-1;
		} else {
			// 被分段
			assert(iEnd+ReserveLen <= _header->iBegin);
			unsigned long len = 0;
			unsigned long new_begin = 0;
			char *data_from = NULL;
			char *data_to = NULL;
			assert(_header->iBegin < _header->iBufSize);
			// 长度字段也被分段
			if(_header->iBegin+MarkLen > _header->iBufSize) { 
				char tmp[16];
				memcpy(tmp,_data+_header->iBegin,_header->iBufSize-_header->iBegin);
				memcpy(tmp+_header->iBufSize-_header->iBegin,_data,_header->iBegin+MarkLen-_header->iBufSize);
				len = GetLen(tmp);
				data_from = _data+(_header->iBegin+MarkLen-_header->iBufSize); //
				new_begin = _header->iBegin+MarkLen-_header->iBufSize+len;
				assert(new_begin <= iEnd);
			} else {
				len = GetLen(_data+_header->iBegin);
				data_from = _data+_header->iBegin+MarkLen;
				if(data_from == _data+_header->iBufSize) data_from = _data;
				if(_header->iBegin+MarkLen+len < _header->iBufSize) { 
					new_begin = _header->iBegin+MarkLen+len;
				} else { // 数据被分段
					new_begin = _header->iBegin+MarkLen+len-_header->iBufSize;
					assert(new_begin <= iEnd);
				}
			}
			data_to = _data+new_begin;

			if(len > buffersize) {
				_header->iBegin = new_begin;
				tmp_num = _header->iNum;
				if(tmp_num > 0) 
					_header->iNum = tmp_num-1;
				throw runtime_error("squeue::pop data is too long to store in the buffer");
			}
			buffersize = len;
			if(data_to > data_from) {
				assert(data_to - data_from == (long)len);
				memcpy(buffer,data_from,len);
			} else {
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

	// 写端使用
	void push(const char *buffer,unsigned long len) throw(runtime_error)
	{
		if(len == 0) return;
		unsigned long iBegin = _header->iBegin;
		if(_header->iEnd == iBegin) {
            _header->iNum = 0;
			if(MarkLen+len+ReserveLen>_header->iBufSize) 
				throw runtime_error("squeue::push full");
		} else if(_header->iEnd > iBegin) {
			assert(iBegin+MarkLen < _header->iEnd);
			if(_header->iBufSize - _header->iEnd + iBegin < MarkLen+len+ReserveLen)
				throw runtime_error("squeue::push full");
		} else {
			assert(_header->iEnd+ReserveLen <= iBegin);
			if(iBegin - _header->iEnd < MarkLen+len+ReserveLen)
				throw runtime_error("squeue::push full");
		}

		// 长度字段被分段
		if(_header->iEnd+MarkLen > _header->iBufSize) {
			char tmp[16]; SetLen(tmp,len);
			memcpy(_data+_header->iEnd,tmp,_header->iBufSize-_header->iEnd);
			memcpy(_data,tmp+_header->iBufSize-_header->iEnd,_header->iEnd+MarkLen-_header->iBufSize);
			memcpy(_data+_header->iEnd+MarkLen-_header->iBufSize,buffer,len);
			_header->iEnd = len+_header->iEnd+MarkLen-_header->iBufSize;
			assert(_header->iEnd+ReserveLen <= iBegin);
			_header->iNum++;
		} 
		// 数据被分段
		else if(_header->iEnd+MarkLen+len > _header->iBufSize){
			SetLen(_data+_header->iEnd,len);
			memcpy(_data+_header->iEnd+MarkLen,buffer,_header->iBufSize-_header->iEnd-MarkLen);
			memcpy(_data,buffer+_header->iBufSize-_header->iEnd-MarkLen,len-(_header->iBufSize-_header->iEnd-MarkLen));
			_header->iEnd = len-(_header->iBufSize-_header->iEnd-MarkLen);
			assert(_header->iEnd+ReserveLen <= iBegin);
			_header->iNum++;
		} else {
			SetLen(_data+_header->iEnd,len);
			memcpy(_data+_header->iEnd+MarkLen,buffer,len);
			_header->iEnd = (_header->iEnd+MarkLen+len)%_header->iBufSize;
			_header->iNum++;
		}
	}

	bool pop(char *buffer1,unsigned long &buffersize1,char *buffer2,unsigned long &buffersize2) throw(runtime_error)
	{
		unsigned long buffersize = buffersize1+buffersize2;
		unsigned long iEnd=_header->iEnd;
		unsigned long tmp_num;
		if(_header->iBegin == iEnd) {
            _header->iNum = 0;
			return false;
        }
		else if(_header->iBegin < iEnd) {
			assert(_header->iBegin+MarkLen < iEnd);
			unsigned long len = GetLen(_data+_header->iBegin);
			assert(_header->iBegin+MarkLen+len <= iEnd);
			if(len > buffersize) {
				_header->iBegin += len+MarkLen;
				tmp_num = _header->iNum;
				if(tmp_num > 0) 
					_header->iNum = tmp_num-1;
				throw runtime_error("squeue::pop data is too long to store in the buffer");
			}
			if(buffersize1 > len) {
				buffersize1 = len;
				buffersize2 = 0;
				memcpy(buffer1,_data+_header->iBegin+MarkLen,len);
			} else {
				buffersize2 = len-buffersize1;
				memcpy(buffer1,_data+_header->iBegin+MarkLen,buffersize1);
				memcpy(buffer2,_data+_header->iBegin+MarkLen+buffersize1,buffersize2);
			}
			_header->iBegin += len+MarkLen;
			tmp_num = _header->iNum;
			if(tmp_num > 0) 
				_header->iNum = tmp_num-1;			
		} else {
			// 被分段
			assert(iEnd+ReserveLen <= _header->iBegin);
			unsigned long len = 0;
			unsigned long new_begin = 0;
			char *data_from = NULL;
			char *data_to = NULL;
			assert(_header->iBegin < _header->iBufSize);
			// 长度字段也被分段
			if(_header->iBegin+MarkLen > _header->iBufSize) { 
				char tmp[16];
				memcpy(tmp,_data+_header->iBegin,_header->iBufSize-_header->iBegin);
				memcpy(tmp+_header->iBufSize-_header->iBegin,_data,_header->iBegin+MarkLen-_header->iBufSize);
				len = GetLen(tmp);
				data_from = _data+(_header->iBegin+MarkLen-_header->iBufSize); //
				new_begin = (_header->iBegin+MarkLen-_header->iBufSize)+len;
				assert(new_begin <= iEnd);
			} else {
				len = GetLen(_data+_header->iBegin);
				data_from = _data+_header->iBegin+MarkLen;
				if(data_from == _data+_header->iBufSize) data_from = _data;
				if(_header->iBegin+MarkLen+len < _header->iBufSize) { 
					new_begin = _header->iBegin+MarkLen+len;
				} else { // 数据被分段
					new_begin = _header->iBegin+MarkLen+len-_header->iBufSize;
					assert(new_begin <= iEnd);
				}
			}
			data_to = _data+new_begin;

			if(len > buffersize) {
				_header->iBegin = new_begin;
				tmp_num = _header->iNum;
				if(tmp_num > 0) 
					_header->iNum = tmp_num-1;
				throw runtime_error("squeue::pop data is too long to store in the buffer");
			}
			buffersize = len;
			if(data_to > data_from) {
				assert(data_to - data_from == (long)len);
				if(buffersize1 > len) {
					buffersize1 = len;
					buffersize2 = 0;
					memcpy(buffer1,data_from,len);
				} else {
					buffersize2 = len-buffersize1;
					memcpy(buffer1,data_from,buffersize1);
					memcpy(buffer2,data_from+buffersize1,buffersize2);
				}
			} else {
				assert(_header->iBufSize-(data_from-data_to)== len);
				if(buffersize1>len) {
					buffersize1 = len;
					buffersize2 = 0;
					memcpy(buffer1,data_from,_data+_header->iBufSize-data_from);
					memcpy(buffer1+(_data+_header->iBufSize-data_from),_data,data_to-_data);
				} else if(buffersize1>_data-data_from+_header->iBufSize) { //buffer1被分段
					buffersize2 = len-buffersize1;
					memcpy(buffer1,data_from,_data+_header->iBufSize-data_from);
					memcpy(buffer1+(_data+_header->iBufSize-data_from),_data,buffersize1-(_data+_header->iBufSize-data_from));
					memcpy(buffer2,data_from+buffersize1-_header->iBufSize,buffersize2);
				} else { //buffer2被分段
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
		if(_header->iEnd == iBegin) {
            _header->iNum = 0;
			if(MarkLen+len+ReserveLen>_header->iBufSize) 
				throw runtime_error("squeue::push full");
		} else if(_header->iEnd > iBegin) {
			assert(iBegin+MarkLen < _header->iEnd);
			if(_header->iBufSize - _header->iEnd + iBegin < MarkLen+len+ReserveLen)
				throw runtime_error("squeue::push full");
		} else {
			assert(_header->iEnd+ReserveLen <= iBegin);
			if(iBegin - _header->iEnd < MarkLen+len+ReserveLen)
				throw runtime_error("squeue::push full");
		}

		// 长度字段被分段
		if(_header->iEnd+MarkLen > _header->iBufSize) {
			char tmp[16]; SetLen(tmp,len);
			memcpy(_data+_header->iEnd,tmp,_header->iBufSize-_header->iEnd);
			memcpy(_data,tmp+_header->iBufSize-_header->iEnd,_header->iEnd+MarkLen-_header->iBufSize);
			memcpy(_data+_header->iEnd+MarkLen-_header->iBufSize,buffer1,len1);
			memcpy(_data+_header->iEnd+MarkLen-_header->iBufSize+len1,buffer2,len2);
			_header->iEnd = len+_header->iEnd+MarkLen-_header->iBufSize;
			assert(_header->iEnd+ReserveLen <= iBegin);
			_header->iNum++;
		} 
		// 数据被分段
		else if(_header->iEnd+MarkLen+len > _header->iBufSize){
			SetLen(_data+_header->iEnd,len);
			if(_header->iEnd+MarkLen+len1>_header->iBufSize) { //buffer1被分段
				memcpy(_data+_header->iEnd+MarkLen,buffer1,_header->iBufSize-_header->iEnd-MarkLen);
				memcpy(_data,buffer1+_header->iBufSize-_header->iEnd-MarkLen,len1-(_header->iBufSize-_header->iEnd-MarkLen));
				memcpy(_data+len1-(_header->iBufSize-_header->iEnd-MarkLen),buffer2,len2);
			} else { //buffer2被分段
				memcpy(_data+_header->iEnd+MarkLen,buffer1,len1);
				memcpy(_data+_header->iEnd+MarkLen+len1,buffer2,_header->iBufSize-_header->iEnd-MarkLen-len1);
				memcpy(_data,buffer2+_header->iBufSize-_header->iEnd-MarkLen-len1,len2-(_header->iBufSize-_header->iEnd-MarkLen-len1));
			}
			_header->iEnd = len-(_header->iBufSize-_header->iEnd-MarkLen);
			assert(_header->iEnd+ReserveLen <= iBegin);
			_header->iNum++;
		} else {
			SetLen(_data+_header->iEnd,len);
			memcpy(_data+_header->iEnd+MarkLen,buffer1,len1);
			memcpy(_data+_header->iEnd+MarkLen+len1,buffer2,len2);
			_header->iEnd = (_header->iEnd+MarkLen+len)%_header->iBufSize;
			_header->iNum++;
		}
	}
	// 读端使用
	bool empty() const {unsigned long iEnd=_header->iEnd;return _header->iBegin == iEnd;}
	// 写端使用
	bool full(unsigned long len) const
	{
		unsigned long iBegin = _header->iBegin;
		if(len==0) return false;

		if(_header->iEnd == iBegin) {
			if(len+MarkLen+ReserveLen > _header->iBufSize) return true;
			return false;
		} else if(_header->iEnd > iBegin) {
			assert(iBegin+MarkLen < _header->iEnd);
			return _header->iBufSize - _header->iEnd + iBegin < MarkLen+len+ReserveLen;
		}
		assert(_header->iEnd+ReserveLen <= iBegin);
		return (iBegin - _header->iEnd < MarkLen+len+ReserveLen);
	}
	// 返回队列里的元素数量，不一定绝对准确，只能作为参考
	unsigned long size() const 
    {
        if (empty()) {
			_header->iNum = 0;
        }
        return _header->iNum;
    } 

private:
	unsigned long GetLen(char *buf) {unsigned long u; memcpy((void *)&u,buf,MarkLen); return u;}
	void SetLen(char *buf,unsigned long u) {memcpy(buf,(void *)&u,MarkLen);}

private:
	const static unsigned long ReserveLen = 8;
	const static unsigned long MarkLen = sizeof(unsigned long);
	struct Header
	{
		unsigned long iBufSize;
		unsigned long iReserveLen; // must be 8
		unsigned long iBegin;
		unsigned long iEnd;
		unsigned long iNum;		// 增加一个数量标记，不一定准确
	};

	Header *_header;
	char *_data;
};

}
#endif
