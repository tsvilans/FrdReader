#ifndef UTIL_H
#define UTIL_H

#include <string>

inline std::string ltrim(std::string& str)
{
	auto it2 = std::find_if(str.begin(), str.end(), [](char ch) { return !std::isspace(static_cast<unsigned char>(ch)); });
	str.erase(str.begin(), it2);
	return str;
}

// trim from end (in place)
inline std::string rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), s.end());
	return s;
}

inline std::string trim(std::string s)
{
	rtrim(s);
	ltrim(s);
	return s;
}

#endif