#ifndef STRING_H
#define STRING_H

#include <errno.h>
#include <stdarg.h>

#include <functional>
#include <algorithm>
#include <cctype>
#include <locale>
#include <string>
#include <sstream>
#include <vector>

namespace String {


	// some functions that help to do some often-used string manipulations
	
	inline bool ToInt(const char *str, int base, int *dest)
	{
		long value;
		char *endPtr;

		errno = 0; // used by strtol()
		value = strtol(str, &endPtr, base);

		if((errno == ERANGE && (value == LONG_MAX || value == LONG_MIN))
			|| (errno != 0 && value == 0))  // range error
		{
			return(false);
		}

		if(endPtr == str) // no digits were found
			return(false);

		if(*endPtr != 0x00) // other characters after int
			return(false);

		// Everything's fine
		*dest = value;

		return(true);
	}

	inline bool ToInt(const std::string &str, int base, int *dest)
	{
		return(ToInt(str.c_str(), base, dest));
	}

	inline bool ToInt(const std::string &str, int &dest, const std::string &hexPrefix)
	{
		size_t prefixLen = hexPrefix.length();

		if(str.length() > prefixLen && str.substr(0, prefixLen) == hexPrefix)
			return(ToInt(str.c_str() + prefixLen, 16, &dest));
		else
			return(ToInt(str.c_str(), 10, &dest));
	}

	inline bool FirstCharLikeNumber(const std::string &str, const char *hexPrefix)
	{
		return(str.size() > 0 && (str.at(0) == hexPrefix[0] || (str.at(0) >= 0x30 && str.at(0) <= 0x39) || str.at(0) == '-'));
	}

	inline std::string IntToStr(int value)
	{
		// this buffer even holds the decimal representation of 64-bit numbers
		char buffer[0x20];
		sprintf(buffer, "%d", value);
		return(buffer);
	}

	inline std::string &Trim(std::string &s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), 
			std::not1(std::ptr_fun<int, int>(std::isspace))));
		s.erase(std::find_if(s.rbegin(), s.rend(), 
			std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());

		return(s);
	}

	inline void Replace(std::string *str, const std::string &from, const std::string &to)
	{
		size_t startPos = 0;
		while((startPos = str->find(from, startPos)) != std::string::npos) {
			str->replace(startPos, from.size(), to);
			startPos += to.size();
		}
	}

	template<typename Container>
	void Tokenize(const std::string &str, Container &tokens, char delimiter)
	{
		std::stringstream ss(str);
		std::string item;
		while(std::getline(ss, item, delimiter))
			if(!item.empty())
				tokens.push_back(item);
	}

	inline std::string Join(std::vector<std::string> &container, std::string delimiter)
	{
		std::string str;
		for(size_t i = 0; i < container.size(); i++) {
			if(i > 0)
				str.append(delimiter);
			str += container[i];
		}

		return(str);
	}
}

#endif /* STRING_H */
