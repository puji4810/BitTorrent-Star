cmake_minimum_required(VERSION 3.27)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

PROJECT(BitTorrent-Star)
set(CMAKE_CXX_STANDARD 23)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(LibtorrentRasterbar REQUIRED)
find_package(spdlog REQUIRED)
find_library(JSONCPP_LIBRARY jsoncpp REQUIRED)

# 添加包含路径
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${LibtorrentRasterbar_INCLUDE_DIRS})
include_directories(${spdlog_INCLUDE_DIRS})
include_directories(${JSONCPP_INCLUDE_DIRS})

configure_file(
        ${CMAKE_SOURCE_DIR}/config.json
        ${CMAKE_BINARY_DIR}/config.json
        COPYONLY
)

# 包含 indicators 的头文件
include_directories(${CMAKE_SOURCE_DIR}/external/indicators/include/)

# 设置默认构建类型为 Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 添加调试标志
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.c)
add_executable(${PROJECT_NAME} ${SRC_FILES}
        utils/Jsonhelper.hpp)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)

target_link_libraries(${PROJECT_NAME} pthread Boost::program_options LibtorrentRasterbar::torrent-rasterbar ${JSONCPP_LIBRARY} spdlog::spdlog_header_only)
