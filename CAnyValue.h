#ifndef __CANY_VALUE_H__
#define __CANY_VALUE_H__

#include <stdexcept>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <memory.h>
#include <assert.h>
#include "Utils.h"
#include <iostream>
#include <deque>
#include <stdint.h>
#include <netinet/in.h>

using namespace std;

namespace lce
{


	struct DType
	{
		enum ValueType	//realtype
		{
			Integer1	= 0,		///< tiny int value (1字节)
			Integer2	= 1,		///< small int value (2字节)
			Integer4	= 2,		///< signed integer value(int32)(4字节)
			Integer8	= 3,		///< big signed interger value(int64)(8字节)
			Integer		= Integer8,
			String1		= 4,		///< string value	//1个字节表示长度
			String2		= 5,		///< string value	//2个字节表示长度
			String4		= 6,		///< string value	//4个字节表示长度
			String		= String4,
			Vector		= 7,		///< array value (double list)
			Map			= 8,		///< object value (collection of name/value pairs).
			EXT			= 9,			///< 协议定义描述等信息
		};

		enum InnerValueType{
			Null = 255,		///< 'null' value
		};
	};

	struct EncodeType{
		enum ENCODETYPE{
			NORMAL,		//正常编码
			TYPE1,		//优化编码
		};
	};

	class CBinary
	{
	public:
		CBinary()
			:m_bDeepCopy(true)
			,m_pData(NULL)
			,m_dwSize(0)
		{}

		CBinary(const char* pData, const size_t dwSize, const bool bDeepCopy=true)
			:m_bDeepCopy(bDeepCopy)
		{
			m_dwSize = dwSize;
			if ( m_bDeepCopy ){
				m_pData = new char[dwSize];
				if ( NULL != m_pData ){
					memcpy(m_pData, pData, dwSize);
				}
			}else{
				m_pData = (char*)pData;
			}
		}

		CBinary(const char* pData)
			:m_bDeepCopy(true)
		{
			m_dwSize = strlen(pData);
			m_pData = new char[m_dwSize];
			if ( NULL != m_pData ){
				memcpy(m_pData, pData, m_dwSize);
			}
		}

		CBinary(const std::string& sData)
			:m_bDeepCopy(true)
		{
			m_pData = new char[sData.size()];
			if ( NULL != m_pData ){
				m_dwSize = sData.size();
				memcpy(m_pData, sData.data(), sData.size());
			}
		}


		~CBinary(){	this->clear();	}

		CBinary(const CBinary& rhs)
			:m_bDeepCopy(rhs.m_bDeepCopy)
			,m_pData(NULL)
			,m_dwSize(rhs.m_dwSize)
		{
			if (m_bDeepCopy){
				if (NULL != rhs.m_pData){
					m_pData = new char[m_dwSize];
					if ( NULL != m_pData ){
						memcpy(m_pData, rhs.m_pData, m_dwSize);
					}
				}
			}
			else{
				m_pData = rhs.m_pData;
			}
		}
		CBinary& operator=(const CBinary& rhs){
			if ( this != &rhs)
			{
				if (m_bDeepCopy && m_pData){
					delete m_pData;
				}
				m_bDeepCopy = rhs.m_bDeepCopy;
				m_dwSize = rhs.m_dwSize;
				if (rhs.m_bDeepCopy)
				{
					if (NULL != rhs.m_pData){
						m_pData = new char[m_dwSize];
						assert( NULL != m_pData );
						memcpy(m_pData, rhs.m_pData, m_dwSize);
					}
				}else{
					m_pData = rhs.m_pData;
				}
			}

			return *this;
		}

		void assign(const char* pData, const size_t dwSize, const bool bDeepCopy=true){
			if (m_bDeepCopy && m_pData){
				delete m_pData;
				m_pData = NULL;
			}

			if (bDeepCopy){
				m_pData = new char[dwSize];
				assert( NULL != m_pData );
				memcpy(m_pData, pData, dwSize);
			}else{
				m_pData = (char*)pData;
			}

			m_dwSize = dwSize;
			m_bDeepCopy = bDeepCopy;
		}
		void clear(){
			if ( m_bDeepCopy && NULL != m_pData){
				delete m_pData;
			}
			m_pData = NULL;
			m_dwSize = 0;
		}

		size_t size() const{	return m_dwSize;	}
		const char* data() const{	return m_pData;	}
	protected:
		friend bool operator==(const CBinary& lhs, const CBinary& rhs);
		friend bool operator!=(const CBinary& lhs, const CBinary& rhs);
		friend bool operator<(const CBinary& lhs, const CBinary& rhs);
	private:
		bool m_bDeepCopy;
		char* m_pData;
		size_t m_dwSize;
	};

	inline bool  operator==(const CBinary& lhs, const CBinary& rhs){
		return  ( lhs.m_dwSize == rhs.m_dwSize && memcmp(lhs.m_pData, rhs.m_pData, rhs.m_dwSize) == 0 ) ? true : false;
	}
	inline bool  operator!=(const CBinary& lhs, const CBinary& rhs){
		return  ( lhs.m_dwSize != rhs.m_dwSize || memcmp(lhs.m_pData, rhs.m_pData, rhs.m_dwSize) != 0 ) ? true : false;
	}
	inline bool operator<(const CBinary& lhs, const CBinary& rhs){
		bool bOk = false;
		if ( lhs.m_dwSize < rhs.m_dwSize )
		{
			bOk = ( memcmp(lhs.m_pData, rhs.m_pData, lhs.m_dwSize) <= 0 ) ? true : false;
		}
		else
		{
			bOk = ( memcmp(lhs.m_pData, rhs.m_pData, rhs.m_dwSize) < 0 ) ? true : false;
		}

		return  bOk;
	}


	class CAnyValue{
	public:
		typedef CBinary	BufType;
		typedef CAnyValue this_type;
		typedef std::map<BufType, CAnyValue> MapType;
		typedef std::deque<CAnyValue> VecType;
		typedef std::runtime_error Error;
		typedef std::map<BufType, int> MAPKEY;
		typedef std::vector<BufType> VECKEY;

		struct SEncode{
			SEncode()
				:ucUsingByteCount(1)
				,iKeyValue(0)
				,bNeedTransform(false)
			{}
			unsigned char ucUsingByteCount;		//使用几个字节保存keyvalue信息
			int iKeyValue;						//优化使用的key值
			bool bNeedTransform;			//是否需要转换
			MAPKEY mapKeys;						//string key
			void clear(){
				mapKeys.clear();
				bNeedTransform = false;
				iKeyValue = 0;
				ucUsingByteCount = 1;
			}
		};

		struct SDecode{
			SDecode()
				:ucUsingByteCount(1)
				,bNeedTransform(false)
				,ucValueType(DType::Null)
			{
			};
			unsigned char ucUsingByteCount;		//使用几个字节保存keyvalue信息
			bool bNeedTransform;			//是否需要转换
			unsigned char ucValueType;
			VECKEY vecKeys;
			void clear(){
				ucUsingByteCount = 1;
				bNeedTransform = false;
				ucValueType = DType::Null;
				vecKeys.clear();
			}
		};

		union ValueHolder{
			uint64_t integer;
			BufType* buf;
			VecType* vec;
			MapType* map;
		};

		typedef void (*ENCODE_FUNC)(std::string&, const ValueHolder&);
		typedef void (*DECODE_FUNC)(size_t& , const char* , const size_t , this_type& );

//		static std::map<int*, std::vector<std::string> > s_map_vec_Info;

		//static void Print(){
		//	cout << "##############################Print start#############################" << endl;
		//	for ( std::map<int*, std::vector<std::string> >::const_iterator it=s_map_vec_Info.begin(); it!=s_map_vec_Info.end(); ++it )
		//	{
		//		printf("0x%x:", it->first);
		//		for ( std::vector<std::string>::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2 )
		//		{
		//			cout << *it2 ;
		//		}
		//		cout << endl;
		//	}
		//	cout << "##############################Print end#############################" << endl;
		//}
	public:
		CAnyValue()
			:m_ucType(DType::Null)
			,m_ucSubType(DType::Null)
			,m_bInit(false)
			,m_bHasData(false)
		{
			init();
			m_value.integer = 0;
		}
		CAnyValue(const char cValue)
			:m_ucType(DType::Integer)
			,m_ucSubType(DType::Integer1)
			,m_bInit(false)
			,m_bHasData(cValue==0?false:true)
		{
			init();
			m_value.integer = cValue;
		}

		CAnyValue(const bool bValue)
			:m_ucType(DType::Integer)
			,m_ucSubType(DType::Integer1)
			,m_bInit(false)
			,m_bHasData(bValue)
		{
			init();
			m_value.integer = bValue?1:0;
		}

		CAnyValue(const unsigned char ucValue)
			:m_ucType(DType::Integer)
			,m_ucSubType(DType::Integer1)
			,m_bInit(false)
			,m_bHasData(ucValue==0?false:true)
		{
			init();
			m_value.integer = ucValue;
		}
		CAnyValue(const unsigned short wValue)
			:m_ucType(DType::Integer)
			,m_ucSubType(DType::Integer2)
			,m_bInit(false)
			,m_bHasData(wValue==0?false:true)
		{
			init();
			m_value.integer = wValue;
		}
		CAnyValue(const short shValue)
			:m_ucType(DType::Integer)
			,m_ucSubType(DType::Integer2)
			,m_bInit(false)
			,m_bHasData(shValue==0?false:true)
		{
			init();
			m_value.integer = shValue;
		}

        #ifdef __x86_64__

		CAnyValue(const long lValue)
			:m_ucType(DType::Integer)
			,m_ucSubType(DType::Integer8)
			,m_bInit(false)
			,m_bHasData(lValue==0?false:true)
		{
			init();
			m_value.integer = lValue;
		}
		CAnyValue(const unsigned long dwValue)
			:m_ucType(DType::Integer)
			,m_ucSubType(DType::Integer8)
			,m_bInit(false)
			,m_bHasData(dwValue==0?false:true)
		{
			init();
			m_value.integer = dwValue;
		}
		#else
		CAnyValue(const long lValue)
			:m_ucType(DType::Integer)
			,m_ucSubType(DType::Integer4)
			,m_bInit(false)
			,m_bHasData(lValue==0?false:true)
		{
			init();
			m_value.integer = lValue;
		}
		CAnyValue(const unsigned long dwValue)
			:m_ucType(DType::Integer)
			,m_ucSubType(DType::Integer4)
			,m_bInit(false)
			,m_bHasData(dwValue==0?false:true)
		{
			init();
			m_value.integer = dwValue;
		}

		#endif

		CAnyValue(const int iValue)
			:m_ucType(DType::Integer)
			,m_ucSubType(DType::Integer4)
			,m_bInit(false)
			,m_bHasData(iValue==0?false:true)
		{
			init();
			m_value.integer = iValue;
		}
		CAnyValue(const unsigned int uiValue)
			:m_ucType(DType::Integer)
			,m_ucSubType(DType::Integer4)
			,m_bInit(false)
			,m_bHasData(uiValue==0?false:true)
		{
			init();
			m_value.integer = uiValue;
		}

        //64位系统冲突
        #ifndef __x86_64__

		CAnyValue(const uint64_t ullValue)
			:m_ucType(DType::Integer)
			,m_ucSubType(DType::Integer8)
			,m_bInit(false)
			,m_bHasData(ullValue==0?false:true)
		{
			init();
			m_value.integer = ullValue;
		}

		CAnyValue(const int64_t llValue)
			:m_ucType(DType::Integer)
			,m_ucSubType(DType::Integer8)
			,m_bInit(false)
			,m_bHasData(llValue==0?false:true)
		{
			init();
			m_value.integer = llValue;
		}

        #endif

		CAnyValue(const char* pszValue)
			:m_ucType(DType::String)
			,m_ucSubType(DType::String4)
			,m_bInit(false)
			,m_bHasData(false)
		{
			init();
			m_value.buf = new BufType(pszValue);
			if ( m_value.buf->size() > 0 )
			{
				m_bHasData = true;
			}
			//if ( m_buf->size() <= 0xFF )
			//	m_ucSubType = DType::String1;
			//else if ( m_buf->size() <= 0xFFFF )
			//	m_ucSubType = DType::String2;
			//else
			//	m_ucSubType = DType::String4;

		}

		CAnyValue(const std::string& sValue)
			:m_ucType(DType::String)
			,m_ucSubType(DType::String4)
			,m_bInit(false)
			,m_bHasData(false)
		{
			init();
			m_value.buf  = new BufType(sValue);
			if ( m_value.buf->size() > 0 )
			{
				m_bHasData = true;
			}
		}

		CAnyValue(const CAnyValue& rhs)
			:m_bInit(false)
		{
			init();
			assign(rhs);
			//m_bHasData = rhs.m_bHasData;
			//m_ucSubType = rhs.m_ucSubType;
			//m_ucType = rhs.m_ucType;
			//m_value = rhs.m_value;
		}
		CAnyValue& operator=(const CAnyValue& rhs){
			if ( this != &rhs )
			{
				assign(rhs);
			}

			return *this;
		}

	public:

		int asInt() const {
			if ( DType::Integer == m_ucType )	return static_cast<int>(m_value.integer);
			return 0;
		}

		std::string asString() const {
			if ( DType::String == m_ucType && NULL != m_value.buf  )		return std::string(m_value.buf->data(), m_value.buf->size());
			return m_strNull;
		}

		operator uint64_t() const {
			if ( DType::Integer == m_ucType )	return m_value.integer;
			return 0;
		}
		operator int64_t() const {
			if ( DType::Integer == m_ucType )	return static_cast<int64_t>(m_value.integer);
			return 0;
		}
		operator unsigned char() const {
			if ( DType::Integer == m_ucType )	return static_cast<unsigned char>(m_value.integer);
			return 0;
		}
		//operator char() const {
		//	if ( DType::Integer == m_ucType )	return static_cast<char>(m_value.integer);
		//	return 0;
		//}
		operator unsigned short() const {
			if ( DType::Integer == m_ucType )	return static_cast<unsigned short>(m_value.integer);
			return 0;
		}
		operator short() const {
			if ( DType::Integer == m_ucType )	return static_cast< short>(m_value.integer);
			return 0;
		}
		operator unsigned long() const {
			if ( DType::Integer == m_ucType )	return static_cast<unsigned long>(m_value.integer);
			return 0;
		}
		operator long() const {
			if ( DType::Integer == m_ucType )	return static_cast<long>(m_value.integer);
			return 0;
		}
		operator int() const {
			if ( DType::Integer == m_ucType )	return static_cast<int>(m_value.integer);
			return 0;
		}
		operator unsigned int() const {
			if ( DType::Integer == m_ucType )	return static_cast<unsigned int>(m_value.integer);
			return 0;
		}
		operator bool() const {
			if ( DType::Integer == m_ucType )	return m_value.integer==0 ? false : true;
			return false;
		}
		//operator const std::string() const {
		//	if ( DType::String == m_ucType && NULL != m_value.buf  )		return std::string(m_value.buf->data(), m_value.buf->size());
		//	return m_strNull;
		//}

		operator std::string() const {
			if ( DType::String == m_ucType && NULL != m_value.buf  )		return std::string(m_value.buf->data(), m_value.buf->size());
			return m_strNull;
		}

		const char* data() const {
			if ( DType::String == m_ucType && NULL != m_value.buf  )		return m_value.buf->data();
			return m_strNull.data();

		}
		const size_t size()	const {
			if ( m_ucType == DType::String ){
				if ( NULL != m_value.buf ){
					return m_value.buf->size();
				}
				else{
					assert(false);
				}
			}
			else if ( m_ucType == DType::Vector ){
				if ( NULL != m_value.vec )
					return m_value.vec->size();
				else
					assert(false);
			}
			else if ( m_ucType == DType::Map ){
				if ( NULL != m_value.map )
					return m_value.map->size();
				else
					assert(false);
			}
			return 0;
		}

	public:
		const CAnyValue& operator[](const int iIndex) const {
			if ( (DType::Vector==m_ucType) && NULL != m_value.vec )
			{
				if ( iIndex < (int)m_value.vec->size() )
					return (*m_value.vec)[iIndex];
			}
			return m_null;
		}
		CAnyValue& operator[](const int iIndex) {
			if ( (DType::Vector==m_ucType) && NULL != m_value.vec )
			{
				if ( iIndex < (int)m_value.vec->size() )
					return (*m_value.vec)[iIndex];
				else throw Error("operator[inx]: index invalid");
			}
			else throw Error("operator[inx]: type invalid");
			//		return m_null;
		}
		const CAnyValue& operator[](const std::string& sName) const
		{
			if ( (DType::Map == m_ucType) && NULL != m_value.map )
			{
				MapType::iterator it = m_value.map->find(sName);
				if ( it != m_value.map->end() )
					return it->second;
			}
			return m_null;
		}

		CAnyValue& operator[](const std::string& sName)
		{
			this->InitAsMap();
			if ( m_value.map == NULL )
			{
				throw Error("anyValue type error: no map type.");
			}
			MapType::iterator it = m_value.map->find(sName);
			if ( it == m_value.map->end() )
			{
				m_bHasData = true;

				if ( sName.size() > 255 )
					it = m_value.map->insert( MapType::value_type(sName.substr(0,255), CAnyValue()) ).first;
				else
					it = m_value.map->insert( MapType::value_type(sName, CAnyValue()) ).first;
			}
			return it->second;
		}

		CAnyValue& operator[](const char* pszName)
		{
			std::string sName(pszName);
			this->InitAsMap();
			if ( m_value.map == NULL )
			{
				throw Error("anyValue type error: no map type.");
			}
			MapType::iterator it = m_value.map->find(sName);
			if ( it == m_value.map->end() )
			{
				m_bHasData = true;
				if ( sName.size() > 255 )
					it = m_value.map->insert( MapType::value_type(sName.substr(0,255), CAnyValue()) ).first;
				else
					it = m_value.map->insert( MapType::value_type(sName, CAnyValue()) ).first;
			}
			return it->second;
		}

		bool hasKey(const std::string& sName) const {
			if ( (DType::Map == m_ucType) && NULL != m_value.map )
			{
				MapType::iterator it = m_value.map->find(sName);
				if ( it != m_value.map->end() )
					return true;
			}
			return false;
		}

		//map
		void insert(const std::string& sName, const CAnyValue& oValue)
		{
			this->InitAsMap();
			if ( (DType::Map == m_ucType) && NULL != m_value.map )
			{
				m_bHasData = true;
				if ( sName.size() > 255 )
					(*m_value.map)[sName.substr(0, 255)] = oValue;
				else
					(*m_value.map)[sName] = oValue;
			}
		}

		//vector
		void push_back(const CAnyValue& oValue){
			this->InitAsVector();
			if ( (DType::Vector == m_ucType) && NULL != m_value.vec )
			{
				m_bHasData = true;
				m_value.vec->push_back(oValue);
			}
		}
		void pop_back(){
			this->InitAsVector();
			if ( (DType::Vector == m_ucType) && NULL != m_value.vec )
			{
				m_bHasData = true;
				m_value.vec->pop_back();
			}
		}
		void push_front(const CAnyValue& oValue){
			this->InitAsVector();
			if ( (DType::Vector == m_ucType) && NULL != m_value.vec )
			{
				m_bHasData = true;
				m_value.vec->push_front(oValue);
			}
		}
		void pop_front(){
			this->InitAsVector();
			if ( (DType::Vector == m_ucType) && NULL != m_value.vec )
			{
				m_bHasData = true;
				m_value.vec->pop_front();
			}
		}

		void erase(const std::string& sName)
		{
			if ( (DType::Map == m_ucType) && NULL != m_value.map )
			{
				m_value.map->erase(sName);
			}
		}

		void clear(){

//			switch(m_ucType)
//			{
//			case DType::String:
//				if ( NULL != m_value.buf ){
////					m_value.buf->clear();
//					delete m_value.buf;
//					m_value.buf = NULL;
//				}
//				break;
//			case DType::Vector:
//				if ( NULL != m_value.vec ){
////					m_value.vec->clear();
//					delete m_value.vec;
//					m_value.vec = NULL;
//				}
//				break;
//			case DType::Map:
//				if ( NULL != m_value.map ){
////					m_value.map->clear();
//					delete m_value.map;
//					m_value.map = NULL;
//				}
//				break;
//			}
//			m_bHasData = false;
//			m_value.integer = 0;
//			m_ucType = DType::Null;
//			m_ucSubType = DType::Null;

			if ( m_bInit ){
				switch(m_ucType)
				{
				case DType::String:
					if ( NULL != m_value.buf ){
						delete m_value.buf;
						m_value.buf = NULL;
					}
					break;
				case DType::Vector:
					if ( NULL != m_value.vec ){
						delete m_value.vec;
						m_value.vec = NULL;
					}
					break;
				case DType::Map:
					if ( NULL != m_value.map ){
						delete m_value.map;
						m_value.map = NULL;
					}
					break;
				}
			}else{
				assert(false);
			}

			memset(&m_value, 0, sizeof(m_value));
			m_ucType = DType::Null;
			m_ucSubType = DType::Null;
			m_bHasData = false;
		}


		~CAnyValue(){
			this->clear();
		}

		void decode_type1(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, const SDecode& stDecode){
			if ( stDecode.bNeedTransform ){
				m_ucSubType = stDecode.ucValueType;
				*(bool*)&stDecode.bNeedTransform = false;
			}
			else{
				check(dwDecodePos+1, dwDataSize);
				m_ucSubType = *(pData+dwDecodePos);
				dwDecodePos++;
			}
			switch(m_ucSubType){
		case DType::Integer1:
			{
				m_ucType = DType::Integer;
				check(dwDecodePos+1, dwDataSize);
				m_value.integer = *(unsigned char*)(pData+dwDecodePos);
				++dwDecodePos;
			}
			break;
		case DType::Integer2:
			{
				m_ucType = DType::Integer;
				check(dwDecodePos+2, dwDataSize);
				m_value.integer = ntohs(*(unsigned short*)(pData+dwDecodePos));
				dwDecodePos += 2;
			}
			break;
		case DType::Integer4:
			{
				m_ucType = DType::Integer;
				check(dwDecodePos+4, dwDataSize);
				m_value.integer = ntohl(*(unsigned long*)(pData+dwDecodePos));
				dwDecodePos += 4;
			}
			break;
		case DType::Integer8:
			{
				m_ucType = DType::Integer;
				check(dwDecodePos+8, dwDataSize);
				//				m_value.integer = (*(long long*)(pData+dwDecodePos));
				m_value.integer = ntohll(*(uint64_t*)(pData+dwDecodePos));
				dwDecodePos += 8;
			}
			break;
		case DType::String1:
			{
				this->InitAsBuf();
				check(dwDecodePos+1, dwDataSize);
				unsigned char ucStrLen = *(unsigned char*)(pData+dwDecodePos);
				++dwDecodePos;
				check(dwDecodePos+ucStrLen, dwDataSize);
				m_value.buf->assign(pData+dwDecodePos, ucStrLen, true);
				dwDecodePos += ucStrLen;
			}
			break;
		case DType::String2:
			{
				this->InitAsBuf();
				check(dwDecodePos+2, dwDataSize);
				unsigned short wStrLen = ntohs(*(unsigned short*)(pData+dwDecodePos));
				dwDecodePos += 2;
				check(dwDecodePos+wStrLen, dwDataSize);
				m_value.buf->assign(pData+dwDecodePos, wStrLen, true);
				dwDecodePos += wStrLen;
			}
			break;
		case DType::String4:
			{
				this->InitAsBuf();
				check(dwDecodePos+4, dwDataSize);
				unsigned long dwStrLen = ntohl(*(unsigned long*)(pData+dwDecodePos));
				dwDecodePos += 4;
				check(dwDecodePos+dwStrLen, dwDataSize);
				m_value.buf->assign(pData+dwDecodePos, dwStrLen, true);
				dwDecodePos += dwStrLen;
			}
			break;
		case DType::Vector:
			{
				this->InitAsVector();
				check(dwDecodePos+4, dwDataSize);
				unsigned long dwSize = ntohl(*(unsigned long*)(pData+dwDecodePos));
				dwDecodePos += 4;
				while ( dwSize > 0 )
				{
					--dwSize;
					CAnyValue value;
					value.decode_type1(dwDecodePos, pData, dwDataSize, stDecode);
					m_value.vec->push_back(value);

				}
			}
			break;
		case DType::Map:
			{
				this->InitAsMap();
				check(dwDecodePos+4, dwDataSize);
				unsigned long dwSize = ntohl(*(unsigned long*)(pData+dwDecodePos));
				dwDecodePos += 4;
				while ( dwSize > 0 )
				{
					--dwSize;
					//unsigned char ucNameLen =  *(unsigned char*)(pData+dwDecodePos);
					//++dwDecodePos;
					//check(dwDecodePos+ucNameLen, dwDataSize);
					//std::string sName(pData+dwDecodePos, ucNameLen);
					//dwDecodePos += ucNameLen;
					unsigned long dwKeyType = 0;
					if ( 1 == stDecode.ucUsingByteCount ){
						check(dwDecodePos+1, dwDataSize);
						dwKeyType = *(unsigned char*)(pData+dwDecodePos);
						++dwDecodePos;
					}else if( 2==stDecode.ucUsingByteCount ){
						check(dwDecodePos+2, dwDataSize);
						dwKeyType = ntohs(*(unsigned short*)(pData+dwDecodePos));
						dwDecodePos += 2;
					}else if ( 4==stDecode.ucUsingByteCount ){
						check(dwDecodePos+4, dwDataSize);
						dwKeyType = ntohl(*(unsigned long*)(pData+dwDecodePos));
						dwDecodePos += 4;
					}else{
						assert(false);
						throw Error("stDecode.ucUsingByteCount error.");
					}

					*(unsigned char*)&stDecode.ucValueType = static_cast<unsigned char>(dwKeyType%10);

					unsigned long dwKeyValue = dwKeyType/10;
					assert(dwKeyValue < stDecode.vecKeys.size());
					if ( dwKeyValue >= stDecode.vecKeys.size() )
						throw Error("dwKeyValue >= stDecode.vecKeys.size()");

					BufType sName(stDecode.vecKeys[dwKeyType/10]);

					*(bool*)&stDecode.bNeedTransform = true;
					CAnyValue value;
					value.decode_type1(dwDecodePos, pData, dwDataSize, stDecode);
					m_value.map->insert( MapType::value_type(sName, value) );
					*(bool*)&stDecode.bNeedTransform = false;
				}

			}
			break;
		case DType::EXT:
			{
				//check(dwDecodePos+1, dwDataSize);
				//unsigned char ucType = *(unsigned char*)(pData+dwDecodePos);
				//++dwDecodePos;
			}
			break;
		default:
			break;
			}
		}

		static void decode_integer1(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
			thisobj.m_ucType = DType::Integer;
			check(dwDecodePos+1, dwDataSize);
			thisobj.m_value.integer = (unsigned char)*(unsigned char*)(pData+dwDecodePos);
			++dwDecodePos;

		}
		static void decode_integer2(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
			thisobj.m_ucType = DType::Integer;
			check(dwDecodePos+2, dwDataSize);
			thisobj.m_value.integer = ntohs(*(unsigned short*)(pData+dwDecodePos));
			dwDecodePos += 2;
		}
		static void decode_integer4(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
			thisobj.m_ucType = DType::Integer;
			check(dwDecodePos+4, dwDataSize);
			thisobj.m_value.integer = ntohl(*(unsigned long*)(pData+dwDecodePos));
			dwDecodePos += 4;
		}
		static void decode_integer8(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
			thisobj.m_ucType = DType::Integer;
			check(dwDecodePos+8, dwDataSize);
			//		thisobj.m_value.integer = (*(long long*)(pData+dwDecodePos));

			thisobj.m_value.integer = ntohll(*(uint64_t*)(pData+dwDecodePos));
			dwDecodePos += 8;
		}
		static void decode_string1(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
			thisobj.InitAsBuf();
			check(dwDecodePos+1, dwDataSize);
			unsigned char ucStrLen = *(unsigned char*)(pData+dwDecodePos);
			++dwDecodePos;
			check(dwDecodePos+ucStrLen, dwDataSize);
			thisobj.m_value.buf->assign(pData+dwDecodePos, ucStrLen, true);
			dwDecodePos += ucStrLen;
		}
		static void decode_string2(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
			thisobj.InitAsBuf();
			check(dwDecodePos+2, dwDataSize);
			unsigned short wStrLen = ntohs(*(unsigned short*)(pData+dwDecodePos));
			dwDecodePos += 2;
			check(dwDecodePos+wStrLen, dwDataSize);
			thisobj.m_value.buf->assign(pData+dwDecodePos, wStrLen, true);
			dwDecodePos += wStrLen;
		}
		static void decode_string4(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
			thisobj.InitAsBuf();
			check(dwDecodePos+4, dwDataSize);
			unsigned long dwStrLen = ntohl(*(unsigned long*)(pData+dwDecodePos));
			dwDecodePos += 4;
			check(dwDecodePos+dwStrLen, dwDataSize);
			thisobj.m_value.buf->assign(pData+dwDecodePos, dwStrLen, true);
			dwDecodePos += dwStrLen;
		}
		static void decode_vector(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
			thisobj.InitAsVector();
			check(dwDecodePos+4, dwDataSize);
			unsigned long dwSize = ntohl(*(unsigned long*)(pData+dwDecodePos));
			dwDecodePos += 4;
			while ( dwSize > 0 )
			{
//				cout << "dwSize=" << dwSize << endl;
				--dwSize;
				CAnyValue value;
				value.decode(dwDecodePos, pData, dwDataSize);
				thisobj.m_value.vec->push_back(value);

			}
		}
		static void decode_map(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
			thisobj.InitAsMap();
			check(dwDecodePos+4, dwDataSize);
			unsigned long dwSize = ntohl(*(unsigned long*)(pData+dwDecodePos));
			dwDecodePos += 4;
			while ( dwSize > 0 )
			{
				--dwSize;
				check(dwDecodePos+1, dwDataSize);
				unsigned char ucNameLen =  *(unsigned char*)(pData+dwDecodePos);
				++dwDecodePos;
				check(dwDecodePos+ucNameLen, dwDataSize);
				BufType sName(pData+dwDecodePos, ucNameLen, true);
				dwDecodePos += ucNameLen;
//				cout << std::string(sName.data(), sName.size()) << endl;
				if ( dwDataSize > dwDecodePos )
				{
					CAnyValue value;
					value.decode(dwDecodePos, pData, dwDataSize);
					thisobj.m_value.map->insert( MapType::value_type(sName, value) );
				}
			}
		}
		static void decode_ext(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
			//check(dwDecodePos+1, dwDataSize);
			//unsigned char ucType = *(unsigned char*)(pData+dwDecodePos);
			//++dwDecodePos;

//			assert(false);
		}

		void decode(size_t& dwDecodePos, const char* pData, const size_t dwDataSize){
			static DECODE_FUNC arDecodeFunc[10] ={&decode_integer1, &decode_integer2, &decode_integer4, &decode_integer8,&decode_string1,
				&decode_string2, &decode_string4, &decode_vector, &decode_map,  &decode_ext};
			check(dwDecodePos+1, dwDataSize);
			m_ucSubType = *(pData+dwDecodePos);
			dwDecodePos++;

			if ( m_ucSubType< 10 ){
				arDecodeFunc[m_ucSubType](dwDecodePos, pData, dwDataSize, *this);
			}


			//switch(m_ucSubType){
			//case DType::Integer1:
			//	decode_integer1(dwDecodePos, pData, dwDataSize, *this);
			//	break;
			//case DType::Integer2:
			//	decode_integer2(dwDecodePos, pData, dwDataSize, *this);
			//	break;
			//case DType::Integer4:
			//	decode_integer4(dwDecodePos, pData, dwDataSize, *this);
			//	break;
			//case DType::Integer8:
			//	decode_integer8(dwDecodePos, pData, dwDataSize, *this);
			//	break;
			//case DType::String1:
			//	decode_string1(dwDecodePos, pData, dwDataSize, *this);
			//	break;
			//case DType::String2:
			//	decode_string2(dwDecodePos, pData, dwDataSize, *this);
			//	break;
			//case DType::String4:
			//	decode_string4(dwDecodePos, pData, dwDataSize, *this);
			//	break;
			//case DType::Vector:
			//	decode_vector(dwDecodePos, pData, dwDataSize, *this);
			//	break;
			//case DType::Map:
			//	decode_map(dwDecodePos, pData, dwDataSize, *this);
			//	break;
			//default:
			//	break;
			//}
		}


		void encode_type1(std::string& sBuf, SEncode& stEncode){
			switch(m_ucType){
		case DType::Integer:
			{
				if ( m_value.integer <= 0xFF ){
					writeKeyType(sBuf, stEncode, DType::Integer1);
					sBuf.push_back((char)m_value.integer);
				}
				else if ( m_value.integer <= 0xFFFF ){
					writeKeyType(sBuf, stEncode, DType::Integer2);
					unsigned short wTmp = htons(static_cast<unsigned short>(m_value.integer));
					sBuf.append(reinterpret_cast<char*>(&wTmp),sizeof(wTmp));
				}
				else if ( m_value.integer <= 0xFFFFFFFF ){
					writeKeyType(sBuf, stEncode, DType::Integer4);
					unsigned long dwTmp = htonl(static_cast<unsigned long>(m_value.integer));
					sBuf.append(reinterpret_cast<char*>(&dwTmp),sizeof(dwTmp));
				}
				else{
					writeKeyType(sBuf, stEncode, DType::Integer8);
					//					sBuf.append(reinterpret_cast<char*>(&m_value.integer),sizeof(m_value.integer));
					uint64_t ui64Tmp = htonll(m_value.integer);
					sBuf.append(reinterpret_cast<char*>(&ui64Tmp),sizeof(ui64Tmp));
				}
			}
			break;
		case DType::String:
			{
				if ( m_value.buf->size() <= 0xFF ){
					writeKeyType(sBuf, stEncode, DType::String1);
					sBuf.push_back(static_cast<char>(m_value.buf->size()));
				}
				else if ( m_value.buf->size() <= 0xFFFF ){
					writeKeyType(sBuf, stEncode, DType::String2);
					unsigned short wSize = htons(static_cast<unsigned short>(m_value.buf->size()));
					sBuf.append(reinterpret_cast<char*>(&wSize), sizeof(wSize));
				}
				else{
					writeKeyType(sBuf, stEncode, DType::String4);
					unsigned long dwSize = htonl(static_cast<unsigned long>(m_value.buf->size()));
					sBuf.append(reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
				}
				sBuf.append(m_value.buf->data(), m_value.buf->size());
			}
			break;
		case DType::Vector:
			{
				writeKeyType(sBuf, stEncode, DType::Vector);
				unsigned long dwSize = htonl(static_cast<unsigned long>(m_value.vec->size()));
				sBuf.append(reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
				for(VecType::iterator it=m_value.vec->begin(); it!=m_value.vec->end(); ++it)
					it->encode_type1(sBuf,stEncode);
			}
			break;
		case DType::Map:
			{
				writeKeyType(sBuf, stEncode, DType::Map);
				unsigned long dwSize = 0;//htonl(static_cast<unsigned long>(m_value.map->size()));
				size_t dwSizePos = sBuf.size();
				sBuf.append(reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
				for(MapType::iterator it=m_value.map->begin(); it!=m_value.map->end(); ++it)
				{
					//sBuf += (char)it->first.size();
					//sBuf += it->first;
					if ( it->second.m_ucType != DType::Null )
					{
						++dwSize;
						stEncode.iKeyValue = stEncode.mapKeys.insert( MAPKEY::value_type( std::string(it->first.data(), it->first.size()), (int)stEncode.mapKeys.size()) ).first->second;
						stEncode.bNeedTransform = true;
						it->second.encode_type1(sBuf, stEncode);
						stEncode.bNeedTransform = false;
					}
				}
				dwSize = htonl(dwSize);
				memcpy((char*)sBuf.data()+dwSizePos, reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
			}
			break;
		default:
			break;
			}
		}

		static void encode_integer(std::string& sBuf, const ValueHolder& value){
			if ( value.integer < 0xFF ){
				sBuf.push_back((char)DType::Integer1);
				sBuf.push_back((char)value.integer);
			}
			else if ( value.integer <= 0xFFFF ){
				sBuf.push_back((char)DType::Integer2);
				unsigned short wTmp = htons(static_cast<unsigned short>(value.integer));
				sBuf.append(reinterpret_cast<char*>(&wTmp),sizeof(wTmp));
			}
			else if ( value.integer <= 0xFFFFFFFF ){
				sBuf.push_back((char)DType::Integer4);
				unsigned long dwTmp = htonl(static_cast<unsigned long>(value.integer));
				sBuf.append(reinterpret_cast<char*>(&dwTmp),sizeof(dwTmp));
			}
			else{
				sBuf.push_back((char)DType::Integer8);
				uint64_t ui64Tmp = htonll(value.integer);
				sBuf.append(reinterpret_cast<char*>(&ui64Tmp),sizeof(ui64Tmp));
			}
		}

		static void encode_string(std::string& sBuf, const ValueHolder& value){
			if ( value.buf->size() <= 0xFF ){
				sBuf.push_back((char)DType::String1);
				sBuf += static_cast<char>(value.buf->size());
			}
			else if ( value.buf->size() <= 0xFFFF ){
				sBuf.push_back((char)DType::String2);
				unsigned short wSize = htons(static_cast<unsigned short>(value.buf->size()));
				sBuf.append(reinterpret_cast<char*>(&wSize), sizeof(wSize));
			}
			else{
				sBuf.push_back((char)DType::String4);
				unsigned long dwSize = htonl(static_cast<unsigned long>(value.buf->size()));
				sBuf.append(reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
			}
			sBuf.append(value.buf->data(), value.buf->size());
		}
		static void encode_vector(std::string& sBuf, const ValueHolder& value){
			sBuf.push_back((char)DType::Vector);
			unsigned long dwSize = htonl(static_cast<unsigned long>(value.vec->size()));
			sBuf.append(reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
			for(VecType::iterator it=value.vec->begin(); it!=value.vec->end(); ++it)
			{
				it->encode(sBuf);
			}
		}
		static void encode_map(std::string& sBuf, const ValueHolder& value){
			sBuf.push_back((char)DType::Map);
			size_t dwSizePos = sBuf.size();
			unsigned long dwSize = 0;//htonl(static_cast<unsigned long>(value.map->size()));
			sBuf.append(reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
			for(MapType::iterator it=value.map->begin(); it!=value.map->end(); ++it)
			{
				if ( it->second.m_ucType != DType::Null )
				{
					++dwSize;
					sBuf.push_back((char)it->first.size());
					sBuf.append(it->first.data(), it->first.size());
					it->second.encode(sBuf);
				}
			}
			dwSize = htonl(dwSize);
			memcpy((char*)sBuf.data()+dwSizePos, reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
		}
		static void encode_none(std::string& sBuf, const ValueHolder& value){
			//sBuf.push_back((char)DType::EXT);
			//char ucType = 0;
			//sBuf += ucType;
//			assert(false);
		}

		void encode(std::string& sBuf){
			//static EnCODE_FUNC arDecodeFunc[12] ={&encode_none, &encode_none, &encode_none, &encode_integer,
			//									  &encode_none, &encode_none, &encode_string, &encode_vector,
			//									  &encode_map,  &encode_none, &encode_none, &encode_none};

			//if ( m_ucType<=8 ){
			//	arEncodeFunc[m_ucType](sBuf, m_value);
			//}

			switch(m_ucType){
		case DType::Integer:
			encode_integer(sBuf, m_value);
			break;
		case DType::String:
			encode_string(sBuf, m_value);
			break;
		case DType::Vector:
			encode_vector(sBuf, m_value);
			break;
		case DType::Map:
			encode_map(sBuf, m_value);
			break;
		default:
			encode_none(sBuf, m_value);
			break;
			}
		}

		void encodeXML(std::string& sBuf,const bool bUtf8=false){
			switch(m_ucType)
			{
			case DType::Integer:
				{
					sBuf += lce::toStr(m_value.integer);
				}
				break;
			case DType::String:
				{
					sBuf += "<![CDATA[";
					sBuf.append(m_value.buf->data(), m_value.buf->size());
					sBuf += "]]>";
				}
				break;
			case DType::Vector:
				{
					for(VecType::iterator it=m_value.vec->begin(); it!=m_value.vec->end(); ++it)
					{
						sBuf += "<l>";
						it->encodeXML(sBuf, bUtf8);
						sBuf += "</l>";
					}
				}
				break;
			case DType::Map:
				{
					for(MapType::iterator it=m_value.map->begin(); it!=m_value.map->end(); ++it)
					{
						sBuf += "<";
						sBuf.append(it->first.data(), it->first.size());
						sBuf += ">";
						it->second.encodeXML(sBuf, bUtf8);
						sBuf += "</";
						sBuf.append(it->first.data(), it->first.size());
						sBuf += ">";
					}
				}
				break;
			default:
				break;
			}
		}

		void encodeJSON(std::string& sBuf, const bool bUtf8=false){
			switch(m_ucType)
			{
			case DType::Integer:
				{
					sBuf += lce::toStr(m_value.integer);
				}
				break;
			case DType::String:
				{
					sBuf += "\"";
					sBuf += lce::textEncodeJSON(string(m_value.buf->data(), m_value.buf->size()), bUtf8);
					sBuf += "\"";
				}
				break;
			case DType::Vector:
				{
					bool bFirst = true;
					sBuf += "[";
					for(VecType::iterator it=m_value.vec->begin(); it!=m_value.vec->end(); ++it)
					{
						if ( !bFirst ){
							sBuf += ",";
						}
						else
						{
							bFirst = false;
						}
						it->encodeJSON(sBuf, bUtf8);
					}
					sBuf += "]";
				}
				break;
			case DType::Map:
				{
					//if ( *sBuf.rbegin() == '}' ){
					//	sBuf += ",{";
					//}
					//else{
					//	sBuf += "{";
					//}
					sBuf += "{";
					bool bFirst = true;
					for(MapType::iterator it=m_value.map->begin(); it!=m_value.map->end(); ++it)
					{
						if ( bFirst ){
							sBuf += "\""; bFirst = false;
						}else{
							sBuf += ",\"";
						}
						//sBuf.append(it->first.data(), it->first.size());
						sBuf += lce::textEncodeJSON(string(it->first.data(), it->first.size()), bUtf8);
						sBuf += "\":";
						it->second.encodeJSON(sBuf, bUtf8);
					}
					sBuf += "}";
				}
				break;
			default:
				{
					sBuf += "\"\"";
				}
				break;
			}
		}


		static std::vector<DECODE_FUNC> m_vecDecodeFuncs;
		static const CAnyValue m_null;
		static const std::string m_strNull;
		static void check(const size_t dwPos, const size_t sDataSize){
			if ( dwPos >  sDataSize ){
				throw Error("decode data error.");
			}
		}
	private:
		bool isHasData() const {	return m_bHasData;	}
		void init(){
			if ( !m_bInit )
			{
				memset(&m_value, 0, sizeof(m_value));
				m_bInit = true;
			}
		}
		void InitAsMap(){
			init();
			assert(DType::Null==m_ucType || DType::Map==m_ucType);
			if ( DType::Null==m_ucType || DType::Map==m_ucType )
			{
				if ( NULL == m_value.map )
				{
					m_ucType = DType::Map;
					m_ucSubType = DType::Map;
					m_value.map = new MapType;
				}
			}
			else
			{
				throw Error("InitAsMap error:type error.");
			}
		}
		void InitAsVector(){
			init();
			assert(DType::Null==m_ucType || DType::Vector==m_ucType);
			if ( DType::Null==m_ucType || DType::Vector==m_ucType )
			{
				if ( NULL == m_value.vec )
				{
					m_ucType = DType::Vector;
					m_ucSubType = DType::Vector;
					m_value.vec = new VecType;
				}
			}
			else
			{
				throw Error("InitAsVector error:type error.");
			}
		}
		void InitAsBuf(){
			init();
			assert(DType::Null==m_ucType || DType::String==m_ucType);
			if ( DType::Null==m_ucType || DType::String==m_ucType )
			{
				if ( NULL == m_value.buf )
				{
					m_ucType = DType::String;
					m_ucSubType = DType::String;
					m_value.buf = new BufType;
				}
			}
			else
			{
				throw Error("InitAsBuf error:type error.");
			}
		}

		void assign(const this_type& rhs){

			if ( m_ucType == DType::String ) delete m_value.buf;
			if ( m_ucType == DType::Vector ) delete m_value.vec;
			if ( m_ucType == DType::Map ) delete m_value.map;

			if ( rhs.m_ucType == DType::Integer ){
				m_value.integer = rhs.m_value.integer;
			}
			else if ( rhs.m_ucType == DType::Map ){
				assert(rhs.m_value.map != NULL);
				m_value.map = new MapType(*(rhs.m_value.map));
			}
			else if ( DType::Vector ==  rhs.m_ucType )
			{
				assert(rhs.m_value.vec != NULL);
				m_value.vec = new VecType(*rhs.m_value.vec);
			}
			else if ( DType::String ==  rhs.m_ucType )
			{
				assert(rhs.m_value.buf != NULL);
				m_value.buf = new BufType((*rhs.m_value.buf));
			}
			m_ucType = rhs.m_ucType;
			m_ucSubType = rhs.m_ucSubType;
			m_bHasData = rhs.m_bHasData;
			m_bInit = rhs.m_bInit;
		}



		void writeKeyType(std::string& sBuf, SEncode& stEncode, const unsigned char ucValueType){
			if ( stEncode.bNeedTransform )
			{
				int iKeyType = stEncode.iKeyValue*10 + ucValueType;
				if ( stEncode.ucUsingByteCount==1 && iKeyType > 255 ){
					stEncode.ucUsingByteCount = 2;
					//write ext type tell to encode using 2 byte.
				}

				if ( stEncode.ucUsingByteCount == 1 ){
					sBuf.push_back((char)iKeyType);
				}
				else if ( stEncode.ucUsingByteCount == 2 ){
					unsigned short wTmp = htons(static_cast<unsigned short>(iKeyType));
					sBuf.append(reinterpret_cast<char*>(&wTmp),sizeof(wTmp));
				}
				else{
					assert(false);
					throw Error("stEncode.ucUsingByteCount error.");
				}

				stEncode.bNeedTransform = false;
			}
			else
			{
				sBuf.push_back((char)ucValueType);
			}
		}

	private:
		unsigned char m_ucType;
		unsigned char m_ucSubType;
		ValueHolder m_value;
		bool m_bInit;
		bool m_bHasData;
	};


	template<typename T>
	class CAnyValuePackage{
	public:
		typedef std::runtime_error Error;
		typedef CAnyValuePackage this_type;
		typedef T PKG_HEAD;
		typedef std::map<CBinary, int> MAPKEY;

		//struct EMode{
		//	enum ENCODEMODE{
		//		ENCODE_AUTO,
		//	};
		//};


    #pragma pack(1)
		struct SEncodeHead{
			SEncodeHead(){	memset(this, 0, sizeof(SEncodeHead));	}
			unsigned char ucKeyTableType:2;	//0:一个字节表示key value的值； 1: 两个字节表示key value的值； 2：四个字节表示key value的值
			unsigned long dwKeyTableOffset;
			unsigned char szReserve[3];
		};
    #pragma pack()

	public:
		CAnyValuePackage(){
			m_sDecodeData.assign(sizeof(PKG_HEAD), 0);
			m_sEncodeData.assign(sizeof(PKG_HEAD), 0);
		}

		CAnyValuePackage(const unsigned char* pszPkgData,const int iLen){
			this->CAnyValuePackage((char*)pszPkgData, iLen);
		}


		CAnyValuePackage(const char* pszPkgData,const int iLen)
		{
			if (NULL == pszPkgData)
			{
				throw Error("pszPkgData is null.");
			}
			if ( iLen < sizeof(PKG_HEAD) )
			{
				throw Error("data no enough len .");
			}
			m_sDecodeData.erase();
			m_sDecodeData.assign((char*)pszPkgData,iLen);
			m_sEncodeData.assign((char*)pszPkgData, sizeof(PKG_HEAD));
		}

		PKG_HEAD& head(){	return *(PKG_HEAD*)m_sEncodeData.data();	}
		void sethead(const PKG_HEAD& stHead)	{
			memcpy((char*)m_sEncodeData.data(), &stHead, sizeof(stHead));
			memcpy((char*)m_sDecodeData.data(), &stHead, sizeof(stHead));
		}

		~CAnyValuePackage(){}

		void encodeJSON(const bool bUtf8=false){
			m_sEncodeData.erase(sizeof(PKG_HEAD));
			m_sEncodeData.assign(m_sDecodeData.data(), sizeof(PKG_HEAD));
			m_oAnyValues.encodeJSON(m_sEncodeData, bUtf8);
		}
		void encodeXML(const bool bUtf8){
			m_sEncodeData.erase(sizeof(PKG_HEAD));
			m_sEncodeData.assign(m_sDecodeData.data(), sizeof(PKG_HEAD));
			if (bUtf8)
				m_sEncodeData += "<?xml version=\"1.0\" encoding=\"UTF-8\"?><anyvalue>";
			else
				m_sEncodeData += "<?xml version=\"1.0\" encoding=\"GB2312\"?><anyvalue>";
			m_oAnyValues.encodeXML(m_sEncodeData);
			m_sEncodeData += "</anyvalue>";
		}


		void encode(const unsigned char ucEncodeType=EncodeType::NORMAL){
			//清除包体数据
			m_sEncodeData.erase(sizeof(PKG_HEAD));
			m_sEncodeData.append((char*)&ucEncodeType, sizeof(ucEncodeType));
			if ( ucEncodeType == EncodeType::TYPE1 )
			{
				//优化编解码
				m_stEncode.clear();
				//encode type1 信息
				SEncodeHead stEncodeHead;
				size_t dwEncodeHeadPos = m_sEncodeData.size();
				m_sEncodeData.append(reinterpret_cast<char*>(&stEncodeHead), sizeof(stEncodeHead));

				//编码
				m_oAnyValues.encode_type1(m_sEncodeData, m_stEncode);

				//write key table;
				stEncodeHead.dwKeyTableOffset = htonl((unsigned long)m_sEncodeData.size());

				//table size
				assert(m_stEncode.mapKeys.size() <= 0xFFFF);
				unsigned short wKeyTableSize = htons((unsigned short)m_stEncode.mapKeys.size());
				m_sEncodeData.append((char*)&wKeyTableSize, sizeof(wKeyTableSize));

				if ( m_stEncode.mapKeys.size() <= 0xFF ){
					stEncodeHead.ucKeyTableType = 0;
					for ( MAPKEY::const_iterator it=m_stEncode.mapKeys.begin(); it!=m_stEncode.mapKeys.end(); ++it )
					{
						assert(it->second <= 0xFF);
						m_sEncodeData += (char)it->second;	//keyvalue
						m_sEncodeData += static_cast<char>(it->first.size());
						m_sEncodeData.append(it->first.data(), it->first.size());			//keyname
					}
				}
				else if ( m_stEncode.mapKeys.size() <= 0xFFFF ){
					stEncodeHead.ucKeyTableType = 1;
					for ( MAPKEY::const_iterator it=m_stEncode.mapKeys.begin(); it!=m_stEncode.mapKeys.end(); ++it )
					{
						assert(it->second <= 0xFFFF);
						unsigned short wKeyValue = htons((unsigned short)it->second);
						m_sEncodeData.append((char*)&wKeyValue, sizeof(wKeyValue));
						m_sEncodeData += static_cast<char>(it->first.size());
						m_sEncodeData.append(it->first.data(), it->first.size());			//keyname
					}
				}
				else{
					assert(false);
					//stEncodeHead.ucKeyTableType = 2;
					//for ( MAPKEY::const_iterator it=m_stEncode.mapKeys.begin(); it!=m_stEncode.mapKeys.end(); ++it )
					//{
					//	unsigned long dwKeyValue = htonl((unsigned long)it->second);
					//	m_sData.append((char*)&dwKeyValue, sizeof(dwKeyValue));
					//	m_sData += static_cast<char>(it->first.size());
					//	m_sData += it->first;
					//}
					throw Error("m_stEncode.mapKeys error: too large.");
				}
				memcpy((char*)(m_sEncodeData.data()+dwEncodeHeadPos), &stEncodeHead, sizeof(stEncodeHead));
			}
			else
			{
				//正常编解码
				m_oAnyValues.encode(m_sEncodeData);
			}
		}

		void decode(const unsigned char* pData, const size_t dwDataSize){
			this->decode((char*)pData, dwDataSize);
		}
		void decode(const char* pData, const size_t dwDataSize){

			if ( dwDataSize < sizeof(PKG_HEAD) ){
				assert(false);
				throw Error("decode error:dwDataSize < sizeof(PKG_HEAD)");
			}
			m_dwPos = 0;
			m_oAnyValues.clear();
			m_sDecodeData.assign(pData, dwDataSize);
			m_sEncodeData.assign(pData, sizeof(PKG_HEAD));
			if ( dwDataSize < sizeof(PKG_HEAD)+1 )
			{
				return ;
			}
			unsigned char ucEncodeType = *(unsigned char*)(m_sDecodeData.data()+sizeof(PKG_HEAD));
			m_dwPos += sizeof(PKG_HEAD)+1;
			if ( ucEncodeType == EncodeType::NORMAL ){
				//正常编解码
				if ( m_dwPos <  m_sDecodeData.size() )
					m_oAnyValues.decode(m_dwPos, m_sDecodeData.data(), m_sDecodeData.size());
			}
			else if ( ucEncodeType == EncodeType::TYPE1 )
			{
				m_stDecode.clear();
				if ( dwDataSize < m_dwPos+sizeof(SEncodeHead) ){
					assert(false);
					throw Error("decode type1 error.");
				}
				//优化编解码
				//read encode head
				SEncodeHead stEncodeHead;
				memcpy(&stEncodeHead, m_sDecodeData.data()+m_dwPos, sizeof(stEncodeHead));
				stEncodeHead.dwKeyTableOffset = ntohl(stEncodeHead.dwKeyTableOffset);

				m_dwPos += sizeof(SEncodeHead);
				decodeKeyTable(stEncodeHead, m_sDecodeData.data()+stEncodeHead.dwKeyTableOffset, m_sDecodeData.size()-stEncodeHead.dwKeyTableOffset);

				if ( m_dwPos <  m_sDecodeData.size() )
					m_oAnyValues.decode_type1(m_dwPos, m_sDecodeData.data(), stEncodeHead.dwKeyTableOffset, m_stDecode);
			}

		}

		const char* data()	{	return m_sEncodeData.data();	}
		size_t size()	{	return m_sEncodeData.size();	}
		const char* bodydata()	{	return m_sEncodeData.data()+sizeof(PKG_HEAD);	}
		const size_t bodysize()	{	return m_sEncodeData.size()-sizeof(PKG_HEAD);	}
		void setbodydata(const unsigned char* pData, const size_t dwSize){		this->setbodydata((char*)pData, dwSize);	}
		void setbodydata(const char* pData, const size_t dwSize){		m_sEncodeData.replace(sizeof(PKG_HEAD),std::string::npos,  pData,dwSize);	}
		const CAnyValue& operator[](const std::string& sName) const
		{
			return m_oAnyValues[sName];
		}

		CAnyValue& operator[](const std::string& sName)
		{
			return m_oAnyValues[sName];
		}

		bool hasKey(const std::string& sName) const {
			return m_oAnyValues.hasKey(sName);
		}
		void insert(const std::string& sName, const CAnyValue& oValue)
		{
			m_oAnyValues.insert(sName, oValue);
		}

		void erase(const std::string& sName)
		{
			m_oAnyValues.erase(sName);
		}

		void clear()	{	m_oAnyValues.clear();	m_sDecodeData.assign(sizeof(PKG_HEAD), 0);	}
		void clearbody(){	m_oAnyValues.clear();	m_sDecodeData.erase(sizeof(PKG_HEAD));	}
		const CAnyValue& root() const {	return m_oAnyValues;	}
		void setroot(const CAnyValue& any)	{	m_oAnyValues = any;	}
	private:
		void assign(const this_type& rhs)
		{
			m_sEncodeData = rhs.m_sEncodeData;
			m_sDecodeData = rhs.m_sDecodeData;
			m_dwPos = rhs.m_dwPos;
		}


		void decodeKeyTable(const SEncodeHead stEncodeHead, const char* pData, const size_t dwDataSize){
			//decode key table
			size_t dwPos = 0;
			unsigned short wKeyTableSize = ntohs(*(unsigned short*)(pData+dwPos));
			dwPos += 2;
			m_stDecode.vecKeys.resize(wKeyTableSize+1);
			if ( 0 == stEncodeHead.ucKeyTableType ){
				for ( unsigned long i=0; i<wKeyTableSize; ++i ){
					CAnyValue::check(dwPos+2, dwDataSize);
					unsigned char ucKeyValue = *(pData+dwPos);
					dwPos += 1;
					unsigned char ucKeyNameLen = *(pData+dwPos);
					dwPos += 1;
					CAnyValue::check(dwPos+ucKeyNameLen, dwDataSize);
					std::string sName(pData+dwPos, ucKeyNameLen);
					dwPos += ucKeyNameLen;

					assert( ucKeyValue < m_stDecode.vecKeys.size());
					if ( ucKeyValue >= m_stDecode.vecKeys.size() )
						throw Error("ucKeyValue >= m_stDecode.vecKeys.size()");
					m_stDecode.vecKeys[ucKeyValue] = sName;
				}
			}
			else if ( 1 == stEncodeHead.ucKeyTableType ){
				for ( unsigned long i=0; i<wKeyTableSize; ++i ){
					CAnyValue::check(dwPos+3, dwDataSize);
					unsigned short wKeyValue = *(unsigned short*)(pData+dwPos);
					dwPos += 2;
					unsigned char ucKeyNameLen = *(pData+dwPos);
					dwPos += 1;
					CAnyValue::check(dwPos+ucKeyNameLen, dwDataSize);
					std::string sName(pData+dwPos, ucKeyNameLen);
					dwPos += ucKeyNameLen;

					assert( wKeyValue < m_stDecode.vecKeys.size());
					if ( wKeyValue >= m_stDecode.vecKeys.size() )
						throw Error("wKeyValue >= m_stDecode.vecKeys.size()");

					m_stDecode.vecKeys[wKeyValue] = sName;
				}
			}
			else {
				assert(false);
				throw Error("stEncodeHead.ucKeyTableType error: no suport.");
			}
		}
	private:
		size_t m_dwPos;
//		std::string m_sData;
		std::string m_sEncodeData;
		std::string m_sDecodeData;

		CAnyValue m_oAnyValues;
		//	CAnyValue::MapType m_mapAnyValues;
		//	MAPKEY m_mapKeys;
		CAnyValue::SEncode m_stEncode;
		CAnyValue::SDecode m_stDecode;
	};




	class CAnyValueRoot{
	public:
		typedef std::runtime_error Error;
		typedef CAnyValueRoot this_type;
		typedef std::map<CBinary, int> MAPKEY;

    #pragma pack(1)
		struct SEncodeHead{
			SEncodeHead(){	memset(this, 0, sizeof(SEncodeHead));	}
			unsigned char ucKeyTableType:2;	//0:一个字节表示key value的值； 1: 两个字节表示key value的值； 2：四个字节表示key value的值
			unsigned long dwKeyTableOffset;
			unsigned char szReserve[3];
		};
    #pragma pack()

	public:
		CAnyValueRoot(){
		}

		CAnyValueRoot(const unsigned char* pszPkgData,const int iLen){
			CAnyValueRoot((char*)pszPkgData, iLen);
		}


		CAnyValueRoot(const char* pszPkgData,const int iLen)
		{
			if (NULL == pszPkgData)
			{
				throw Error("pszPkgData is null.");
			}
			m_sDecodeData.erase();
			m_sDecodeData.assign((char*)pszPkgData,iLen);
		}

		~CAnyValueRoot(){}

		void encodeJSON(const bool bUtf8=false){
			m_sEncodeData.clear();
			m_oAnyValues.encodeJSON(m_sEncodeData, bUtf8);
		}
		void encodeXML(const bool bUtf8=false){
			m_sEncodeData.clear();
			if (bUtf8)
				m_sEncodeData += "<?xml version=\"1.0\" encoding=\"UTF-8\"?><anyvalue>";
			else
				m_sEncodeData += "<?xml version=\"1.0\" encoding=\"GB2312\"?><anyvalue>";

			m_oAnyValues.encodeXML(m_sEncodeData, bUtf8);
			m_sEncodeData += "</anyvalue>";
		}


		void encode(const unsigned char ucEncodeType=EncodeType::NORMAL){
			//清除包体数据
			m_sEncodeData.clear();
			m_sEncodeData.append((char*)&ucEncodeType, sizeof(ucEncodeType));
			if ( ucEncodeType == EncodeType::TYPE1 )
			{
				//优化编解码
				m_stEncode.clear();
				//encode type1 信息
				SEncodeHead stEncodeHead;
				size_t dwEncodeHeadPos = m_sEncodeData.size();
				m_sEncodeData.append(reinterpret_cast<char*>(&stEncodeHead), sizeof(stEncodeHead));

				//编码
				m_oAnyValues.encode_type1(m_sEncodeData, m_stEncode);

				//write key table;
				stEncodeHead.dwKeyTableOffset = htonl((unsigned long)m_sEncodeData.size());

				//table size
				assert(m_stEncode.mapKeys.size() <= 0xFFFF);
				unsigned short wKeyTableSize = htons((unsigned short)m_stEncode.mapKeys.size());
				m_sEncodeData.append((char*)&wKeyTableSize, sizeof(wKeyTableSize));

				if ( m_stEncode.mapKeys.size() <= 0xFF ){
					stEncodeHead.ucKeyTableType = 0;
					for ( MAPKEY::const_iterator it=m_stEncode.mapKeys.begin(); it!=m_stEncode.mapKeys.end(); ++it )
					{
						assert(it->second <= 0xFF);
						m_sEncodeData += (char)it->second;	//keyvalue
						m_sEncodeData += static_cast<char>(it->first.size());
						m_sEncodeData.append(it->first.data(), it->first.size());			//keyname
					}
				}
				else if ( m_stEncode.mapKeys.size() <= 0xFFFF ){
					stEncodeHead.ucKeyTableType = 1;
					for ( MAPKEY::const_iterator it=m_stEncode.mapKeys.begin(); it!=m_stEncode.mapKeys.end(); ++it )
					{
						assert(it->second <= 0xFFFF);
						unsigned short wKeyValue = htons((unsigned short)it->second);
						m_sEncodeData.append((char*)&wKeyValue, sizeof(wKeyValue));
						m_sEncodeData += static_cast<char>(it->first.size());
						m_sEncodeData.append(it->first.data(), it->first.size());			//keyname
					}
				}
				else{
					assert(false);
					throw Error("m_stEncode.mapKeys error: too large.");
				}
				memcpy((char*)(m_sEncodeData.data()+dwEncodeHeadPos), &stEncodeHead, sizeof(stEncodeHead));
			}
			else
			{
				//正常编解码
				m_oAnyValues.encode(m_sEncodeData);
			}
		}

		void decode(const unsigned char* pData, const size_t dwDataSize){
			this->decode((char*)pData, dwDataSize);
		}
		void decode(const char* pData, const size_t dwDataSize){
			m_dwPos = 0;
			m_oAnyValues.clear();
			m_sDecodeData.assign(pData, dwDataSize);
			if ( dwDataSize < 1 )
			{
				return ;
			}
			unsigned char ucEncodeType = *(unsigned char*)(m_sDecodeData.data());
			m_dwPos += 1;
			if ( ucEncodeType == EncodeType::NORMAL ){
				//正常编解码
				if ( m_dwPos <  m_sDecodeData.size() )
					m_oAnyValues.decode(m_dwPos, m_sDecodeData.data(), m_sDecodeData.size());
			}
			else if ( ucEncodeType == EncodeType::TYPE1 )
			{
				m_stDecode.clear();
				if ( dwDataSize < m_dwPos+sizeof(SEncodeHead) ){
					assert(false);
					throw Error("decode type1 error.");
				}
				//优化编解码
				//read encode head
				SEncodeHead stEncodeHead;
				memcpy(&stEncodeHead, m_sDecodeData.data()+m_dwPos, sizeof(stEncodeHead));
				stEncodeHead.dwKeyTableOffset = ntohl(stEncodeHead.dwKeyTableOffset);

				m_dwPos += sizeof(SEncodeHead);
				decodeKeyTable(stEncodeHead, m_sDecodeData.data()+stEncodeHead.dwKeyTableOffset, m_sDecodeData.size()-stEncodeHead.dwKeyTableOffset);

				if ( m_dwPos <  m_sDecodeData.size() )
					m_oAnyValues.decode_type1(m_dwPos, m_sDecodeData.data(), stEncodeHead.dwKeyTableOffset, m_stDecode);
			}

		}

		const char* data()	{	return m_sEncodeData.data();	}
		size_t size()	{	return m_sEncodeData.size();	}
		const CAnyValue& operator[](const std::string& sName) const
		{
			return m_oAnyValues[sName];
		}

		CAnyValue& operator[](const std::string& sName)
		{
			return m_oAnyValues[sName];
		}

		bool hasKey(const std::string& sName) const {
			return m_oAnyValues.hasKey(sName);
		}

		void insert(const std::string& sName, const CAnyValue& oValue)
		{
			m_oAnyValues.insert(sName, oValue);
		}

		void erase(const std::string& sName)
		{
			m_oAnyValues.erase(sName);
		}

		void clear()	{	m_oAnyValues.clear();	m_sDecodeData.clear();	}
		const CAnyValue& root() const {	return m_oAnyValues;	}
		void setroot(const CAnyValue& any)	{	m_oAnyValues = any;	}
	private:
		void assign(const this_type& rhs)
		{
			m_sEncodeData = rhs.m_sEncodeData;
			m_sDecodeData = rhs.m_sDecodeData;
			m_dwPos = rhs.m_dwPos;
		}


		void decodeKeyTable(const SEncodeHead stEncodeHead, const char* pData, const size_t dwDataSize){
			//decode key table
			size_t dwPos = 0;
			unsigned short wKeyTableSize = ntohs(*(unsigned short*)(pData+dwPos));
			dwPos += 2;
			m_stDecode.vecKeys.resize(wKeyTableSize+1);
			if ( 0 == stEncodeHead.ucKeyTableType ){
				for ( unsigned long i=0; i<wKeyTableSize; ++i ){
					CAnyValue::check(dwPos+2, dwDataSize);
					unsigned char ucKeyValue = *(pData+dwPos);
					dwPos += 1;
					unsigned char ucKeyNameLen = *(pData+dwPos);
					dwPos += 1;
					CAnyValue::check(dwPos+ucKeyNameLen, dwDataSize);
					std::string sName(pData+dwPos, ucKeyNameLen);
					dwPos += ucKeyNameLen;

					assert( ucKeyValue < m_stDecode.vecKeys.size());
					if ( ucKeyValue >= m_stDecode.vecKeys.size() )
						throw Error("ucKeyValue >= m_stDecode.vecKeys.size()");
					m_stDecode.vecKeys[ucKeyValue] = sName;
				}
			}
			else if ( 1 == stEncodeHead.ucKeyTableType ){
				for ( unsigned long i=0; i<wKeyTableSize; ++i ){
					CAnyValue::check(dwPos+3, dwDataSize);
					unsigned short wKeyValue = *(unsigned short*)(pData+dwPos);
					dwPos += 2;
					unsigned char ucKeyNameLen = *(pData+dwPos);
					dwPos += 1;
					CAnyValue::check(dwPos+ucKeyNameLen, dwDataSize);
					std::string sName(pData+dwPos, ucKeyNameLen);
					dwPos += ucKeyNameLen;

					assert( wKeyValue < m_stDecode.vecKeys.size());
					if ( wKeyValue >= m_stDecode.vecKeys.size() )
						throw Error("wKeyValue >= m_stDecode.vecKeys.size()");

					m_stDecode.vecKeys[wKeyValue] = sName;
				}
			}
			else {
				assert(false);
				throw Error("stEncodeHead.ucKeyTableType error: no suport.");
			}
		}
	private:
		size_t m_dwPos;
		std::string m_sEncodeData;
		std::string m_sDecodeData;

		CAnyValue m_oAnyValues;
		CAnyValue::SEncode m_stEncode;
		CAnyValue::SDecode m_stDecode;
	};



};



#endif

