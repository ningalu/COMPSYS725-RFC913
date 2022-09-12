#ifndef _COMMON_COMMANDS_H
#define _COMMON_COMMANDS_H

#include <vector>
#include <string>

namespace sftp {
    class Commands {
        public:
            static const char *USER;
            static const char *ACCT;
            static const char *PASS;
            static const char *TYPE;
            static const char *LIST;
            static const char *CDIR;
            static const char *KILL;
            static const char *NAME;
            static const char *DONE;
            static const char *RETR;
            static const char *STOR;
            static const std::vector<const char *> COMMANDS;
            static bool IsValid(char *command);
            static bool IsValid(const std::string& command);
    };
}

#endif