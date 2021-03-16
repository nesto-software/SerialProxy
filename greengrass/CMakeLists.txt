# minimal CMakeLists.txt for the AWS Greengrass SDK for C
cmake_minimum_required(VERSION 2.8)
set(TOOL serialproxy-gg)
LIST(APPEND TOOLS_LINK_LIBS -lzmq -lstdc++ -pthread aws-greengrass-core-sdk-c)

find_package(aws-greengrass-core-sdk-c REQUIRED)

add_executable(${TOOL} main.cpp)
target_sources(${TOOL} PRIVATE ../sersniff.h ../sersniff.cpp ../disp_basic.h ../disp_basic.cpp)
target_link_libraries(${TOOL} ${TOOLS_LINK_LIBS})
target_compile_options(${TOOL} PRIVATE -Werror -Wall -Wextra -pedantic -Wno-write-strings)