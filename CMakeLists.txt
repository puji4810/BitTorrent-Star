cmake_minimum_required(VERSION 3.20)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project(BitTorrent-Star)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 添加外部依赖
include(FetchContent)

# # 添加 spdlog
# FetchContent_Declare(
#     spdlog
#     GIT_REPOSITORY https://github.com/gabime/spdlog.git
#     GIT_TAG v1.12.0
# )
# FetchContent_MakeAvailable(spdlog)

# # 添加 indicators
# FetchContent_Declare(
#     indicators
#     GIT_REPOSITORY https://github.com/p-ranav/indicators.git
#     GIT_TAG v2.3
# )
# FetchContent_MakeAvailable(indicators)

# # 添加 jsoncpp
# FetchContent_Declare(
#     jsoncpp
#     GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
#     GIT_TAG 1.9.5
# )
# FetchContent_MakeAvailable(jsoncpp)

# 添加 libtorrent
# 使用 CONFIG 模式来避免使用 Find 模块，直接使用 LibtorrentRasterbarConfig.cmake
find_package(LibtorrentRasterbar CONFIG REQUIRED)
find_package(spdlog REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

# 源文件
set(SOURCES
    src/main.cpp
    src/downloader.cpp
    src/download_manager.cpp
)

# 头文件
set(HEADERS
    include/downloader.h
    include/download_manager.h
    include/configer.h
)

# 可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/external/indicators/include
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    spdlog::spdlog
    LibtorrentRasterbar::torrent-rasterbar
    jsoncpp_lib
    Boost::program_options
)

# 编译选项
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# 构建时复制 config.json 到 build 目录
configure_file(${CMAKE_SOURCE_DIR}/config.json ${CMAKE_BINARY_DIR}/bin/config.json COPYONLY)
