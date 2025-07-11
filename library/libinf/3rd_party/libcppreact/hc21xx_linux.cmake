set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_BUILD_TYPE MinSizeRel)

set(CMAKE_C_COMPILER $ENV{CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER $ENV{CROSS_COMPILE}g++)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -s -ffunction-sections -fdata-sections -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -s -ffunction-sections -fdata-sections -Os")

set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER CACHE STRING "")
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER CACHE STRING "")
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER CACHE STRING "")

include_directories("${PROJECT_SOURCE_DIR}/../libtbb/include")
link_directories("${PROJECT_SOURCE_DIR}/../libtbb/lib")