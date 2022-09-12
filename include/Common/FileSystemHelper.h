#ifndef _COMMON_FILESYSTEMHELPER_H
#define _COMMON_FILESYSTEMHELPER_H

#include <vector>
#include <string>

std::vector<std::string> GetFileNamesInDirectory(std::string dir);
int GetFileSize(std::string dir);
std::vector<std::pair<std::string, std::string>> GetFileInfoInDirectory(std::string dir);
bool DirExists(const std::string &dir);
bool FileExists(const std::string &dir);
#endif