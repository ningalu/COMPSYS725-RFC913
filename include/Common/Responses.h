#ifndef _COMMON_RESPONSES_H
#define _COMMON_RESPONSES_H

namespace sftp {
    const char *DUMMY_HOST_NAME = "thol600-server";
    const char *POSITIVE_GREETING = "SFTP Service";

    const char *SUCCESS = "+";
    const char *ERROR = "-";
    // This response should also print the number as text
    const char *NUMBER = " ";
    const char *LOGGED_IN = "!";
}

#endif