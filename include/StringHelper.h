#ifndef __LCE_STRING_HELPER_H
#define __LCE_STRING_HELPER_H

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>

using namespace std;

namespace lce
{
	inline void replaceStr(string &str,const string &sFind,const string &sReplace)
	{
		string::size_type pos = 0;
		string::size_type dwLen1 = sFind.size();
		string::size_type dwLen2 = sReplace.size();

		while((pos = str.find(sFind,pos)) != string::npos)
		{
			str.replace(pos,dwLen1,sReplace);
			pos+=dwLen2;
		}
	}
	
	inline bool splitHostPort(const std::string& sSrc, std::string& Host, std::string&Port, const char splitter=':')
	{
		size_t found=sSrc.find_first_of(splitter);
		if(found!=std::string::npos)
		{
			Host = sSrc.substr(0,found);
			Port = sSrc.substr(found+1);
			return true;
		}
		return false;
	}

	inline void splitStr(const string & str, const string & separator,std::vector<string> &result)
	{
		size_t start = 0;
		size_t end = 0;

		while( (end = str.find(separator, start) ) != string::npos )
		{
			result.push_back(str.substr(start, end - start));
			start = end + 1;
		}

		result.push_back(str.substr(start));

	}


	inline size_t getMiddleStr(size_t begPos, const string & str, string & out, const string & beg, const string & end)
	{
		size_t pos = str.find(beg, begPos);
		if(pos == string::npos)
		{
			return string::npos;
		}

		begPos = pos + beg.size();

		pos = str.find(end, begPos);
		if(pos == string::npos)
		{
			return string::npos;
		}

		out = str.substr(begPos, pos - begPos);
		pos += end.size();

		return pos;
	}

	inline size_t getMiddleStr(size_t begPos, const string & str, string & out, size_t beg, const string & end)
	{

		size_t pos = str.find(end, beg);
		if(pos == string::npos)
		{
			return string::npos;
		}

		out = str.substr(beg, pos - beg);
		pos += end.size();

		return pos;
	}

	//去掉字符串2头的空格
	inline void  trimStr(std::string& sSource)
	{
		if ( sSource.size() == 0 )	return;

		std::string sDest = "";
		size_t i = 0;

		for(; i < sSource.size() &&	(sSource[i] == ' '||sSource[i] == '\r'||sSource[i] == '\n'||sSource[i] == '\t'); i++);

		size_t j = sSource.size() - 1;

		if(j > 0)
		{
			for(; j > 0 &&(sSource[j] == ' '||sSource[j] == '\r'||sSource[j] == '\n'||sSource[j] == '\t'); j--);
		}

		if(j >= i)
		{
			sDest = sSource.substr(i, j-i+1);
			sSource = sDest;
		}
		else
		{
			sSource = "";
		}
	}

}

#endif