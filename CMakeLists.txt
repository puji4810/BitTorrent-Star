cmake_minimum_required(VERSION 3.22)

PROJECT(BitTorrent-Star)
set(CMAKE_CXX_STANDARD 23)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(LibtorrentRasterbar REQUIRED)

# 包含 indicators 的头文件
add_subdirectory(external/indicators)
include_directories(external/indicators/include)

# 设置默认构建类型为 Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 添加调试标志
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.c)
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} pthread Boost::program_options LibtorrentRasterbar::torrent-rasterbar)
