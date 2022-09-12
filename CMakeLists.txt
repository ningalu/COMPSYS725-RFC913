cmake_minimum_required(VERSION 3.20.2)
project(COMPSYS725 VERSION 1.0)
add_executable(Server src/Server/main.cpp)
add_executable(Client src/Client/main.cpp)

# target_compile_features(Server PRIVATE CXX_STANDARD 17)
# target_compile_features(Client PRIVATE CXX_STANDARD 17)
set_target_properties(Server PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
set(SERVER Server)

add_subdirectory(./src/Common)

add_custom_command(
    TARGET Server
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy_directory ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${SERVER}>/resources
)

target_include_directories(
    Server
    PUBLIC ./include/Common
)

target_include_directories(
    Client
    PUBLIC ./include/Common
)

target_link_libraries(
    Server
    Common
)

target_link_libraries(
    Client
    Common
)