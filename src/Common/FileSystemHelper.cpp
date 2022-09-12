#include "FileSystemHelper.h"
#include <ctime>
#include <dirent.h>
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>

std::vector<std::string> GetFileNamesInDirectory(std::string dir) {
  std::vector<std::string> filenames;
  DIR *dir_;
  struct dirent *ent;
  if ((dir_ = opendir(dir.c_str())) != NULL) {
    while ((ent = readdir(dir_)) != NULL) {
      filenames.push_back(std::string{ent->d_name});
    }
    closedir(dir_);
  } else {
    throw std::logic_error("File does not exist");
  }
  return filenames;
}

int GetFileSize(std::string dir) {
  struct stat file_info;
  if (stat(dir.c_str(), &file_info) != 0) {
    throw std::logic_error("File could not be opened");
  }
  bool reg = S_ISREG(file_info.st_mode);
  if (reg) {
    return static_cast<int>(file_info.st_size);
  } else {
    return -1;
  }
}

std::vector<std::pair<std::string, std::string>>
GetFileInfoInDirectory(std::string dir) {
  std::vector<std::pair<std::string, std::string>> file_info;
  try {
    auto file_names = GetFileNamesInDirectory(dir);
    for (auto it : file_names) {
      std::string path = dir + it;
      try {
        int size = GetFileSize(path);
        file_info.push_back(
            {it, size >= 0 ? std::to_string(size) + "B" : "Dir"});
      } catch (std::logic_error &err) {
        file_info.push_back({it, "N/A"});
      }
    }
  } catch (std::logic_error &err) {
    throw err;
  }
  return file_info;
}

bool DirExists(const std::string &dir) {
  struct stat dir_info;
  return stat(dir.c_str(), &dir_info) == 0;
}

bool FileExists(const std::string &dir) {
  struct stat file_info;
  if (stat(dir.c_str(), &file_info) != 0) {
    return false;
  }
  return S_ISREG(file_info.st_mode);
}