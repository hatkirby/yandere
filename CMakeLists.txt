cmake_minimum_required (VERSION 3.1)
project (yandere)

find_package(PkgConfig)
pkg_check_modules(yaml-cpp yaml-cpp REQUIRED)
pkg_check_modules(mastodonpp mastodonpp REQUIRED)

include_directories(
  ${mastodonpp_INCLUDE_DIRS}
  ${yaml-cpp_INCLUDE_DIRS})

link_directories(
  ${mastodonpp_LIBRARY_DIRS}
  ${yaml-cpp_LIBRARY_DIRS})

add_executable(yandere yandere.cpp)
set_property(TARGET yandere PROPERTY CXX_STANDARD 17)
set_property(TARGET yandere PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(yandere ${mastodonpp_LIBRARIES} ${yaml-cpp_LIBRARIES})
