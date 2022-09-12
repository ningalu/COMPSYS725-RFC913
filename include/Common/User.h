#include <string>
namespace sftp {
    // The specification makes it sound like multiple
    // accounts can be associated with the same id but
    // it also discusses remote servers without ids
    // and never actually specifies either way so I
    // won't implement it
    struct User {
        User(std::string user, std::string acct, std::string pass);
        User(std::string line);
        User();
        std::string user;
        std::string acct;
        std::string pass;
        bool operator!=(const User& rhs);
        bool operator==(const User& rhs);
    };
}

std::ostream& operator<<(std::ostream& os, const sftp::User& u);