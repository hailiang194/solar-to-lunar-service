cmake_minimum_required(VERSION 3.1)
project(SolarToLunar)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/mongoose)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/am-duong-lich)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/mongoose/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/json)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/am-duong-lich/include)

set(BUILD_EXAMPLES OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Distribution)

if(MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/Distribution)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/Distribution)
endif()

add_executable(SolarToLunar Source/main.cpp)
target_link_libraries(SolarToLunar mongoose am-duong-lich)
