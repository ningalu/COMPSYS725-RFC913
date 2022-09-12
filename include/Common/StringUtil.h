#ifndef _COMMON_STRINGUTIL_H
#define _COMMON_STRINGUTIL_H

#include <string>
#include <vector>

std::string ToUpper(std::string str);
std::vector<std::string> SplitString(std::string str, char delimiter = ' ');

#endif