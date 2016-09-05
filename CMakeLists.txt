cmake_minimum_required (VERSION 3.1)
project (yandere)

set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(vendor/libtwittercpp)
add_subdirectory(vendor/yaml-cpp EXCLUDE_FROM_ALL)

include_directories(vendor/libtwittercpp/src vendor/yaml-cpp/include)
add_executable(yandere yandere.cpp)
set_property(TARGET yandere PROPERTY CXX_STANDARD 11)
set_property(TARGET yandere PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(yandere yaml-cpp twitter++)
