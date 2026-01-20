cmake_minimum_required(VERSION 3.10)
project(TrafficAnalyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Libpcap
find_package(PkgConfig REQUIRED)
pkg_check_modules(PCAP REQUIRED libpcap)

include_directories(include ${PCAP_INCLUDE_DIRS})
link_directories(${PCAP_LIBRARY_DIRS})

# Source files
file(GLOB SOURCES "src/*.cpp")

# Executable
add_executable(traffic_analyzer ${SOURCES})
target_link_libraries(traffic_analyzer ${PCAP_LIBRARIES} pthread)
