cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "" FORCE)

project(KRapp-client
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /W4 /O2" CACHE STRING "" FORCE)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/menu
    ${CMAKE_CURRENT_SOURCE_DIR}/server
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

file(GLOB SRC
   ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/menu/*.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/server/*.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp
)

find_package(Drogon CONFIG REQUIRED)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(Botan REQUIRED)

add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Drogon::Drogon
    Botan::Botan
)