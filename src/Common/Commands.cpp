#include "Commands.h"

#include <cstring>

namespace sftp {
const char *Commands::USER = "USER";
const char *Commands::ACCT = "ACCT";
const char *Commands::PASS = "PASS";
const char *Commands::TYPE = "TYPE";
const char *Commands::LIST = "LIST";
const char *Commands::CDIR = "CDIR";
const char *Commands::KILL = "KILL";
const char *Commands::NAME = "NAME";
const char *Commands::DONE = "DONE";
const char *Commands::RETR = "RETR";
const char *Commands::STOR = "STOR";
const std::vector<const char *> Commands::COMMANDS = {
    USER, ACCT, PASS, TYPE, LIST, CDIR, KILL, NAME, DONE, RETR, STOR};

bool Commands::IsValid(char *command) {
  for (auto it : Commands::COMMANDS) {
    if (strcmp(command, it) == 0) {
      return true;
    }
  }
  return false;
}

bool Commands::IsValid(const std::string &command) {
  for (auto it : Commands::COMMANDS) {
    if (command == it) {
      return true;
    }
  }
  return false;
}
} // namespace sftp