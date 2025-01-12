cmake_minimum_required(VERSION 3.22)

PROJECT(BitTorrent-Star)
set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(LibtorrentRasterbar REQUIRED)

add_executable(${PROJECT_NAME} test.cpp)

target_link_libraries(${PROJECT_NAME} Boost::system Boost::filesystem LibtorrentRasterbar::torrent-rasterbar)