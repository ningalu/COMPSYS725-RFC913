add_library(
    Common
    ./SftpSocketUtil.cpp
    ./StringUtil.cpp
    ./Commands.cpp
    ./User.cpp
    ./FileSystemHelper.cpp
)

target_include_directories(
    Common
    PUBLIC ${CMAKE_SOURCE_DIR}/include/Common
)