#include "StringUtil.h"

#include <algorithm>
#include <cstring>
#include <ctype.h>
#include <sstream>

std::vector<std::string> SplitString(std::string str, char delimiter) {
  std::vector<std::string> strings;
  std::string token;
  std::stringstream stream(str);
  while (std::getline(stream, token, delimiter)) {
    strings.push_back(token);
  }
  return strings;
}

std::string ToUpper(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(), ::toupper);
  return str;
}
