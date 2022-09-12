#include "User.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace sftp {
User::User(std::string user, std::string acct, std::string pass)
    : user(user), acct(acct), pass(pass) {}

User::User(std::string line) {
  if (std::count(line.begin(), line.end(), '|') != 2) {
    throw std::logic_error("Tried to initialise a User with an invalid entry");
  }
  std::stringstream line_stream(line);
  std::string token;
  std::vector<std::string> tokens;
  while (std::getline(line_stream, token, '|')) {
    tokens.push_back(token);
  }

  // getline doesn't generate an empty string when the delimiter is the last
  // character
  if (line[line.size() - 1] == '|') {
    tokens.push_back("");
  }
  user = tokens[0];
  acct = tokens[1];
  pass = tokens[2];
}

User::User() {}

bool User::operator!=(const User &rhs) {
  return !((user == rhs.user) && (acct == rhs.acct) && (pass == rhs.pass));
}
bool User::operator==(const User &rhs) {
  return (user == rhs.user) && (acct == rhs.acct) && (pass == rhs.pass);
}
} // namespace sftp

std::ostream &operator<<(std::ostream &os, const sftp::User &u) {
  os << u.user << "|" << u.acct << "|" << u.pass;
  return os;
}