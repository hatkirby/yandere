cmake_minimum_required (VERSION 3.1)
project (yandere)

add_subdirectory(vendor/mastodonpp)
add_subdirectory(vendor/yaml-cpp EXCLUDE_FROM_ALL)

include_directories(vendor/mastodonpp/include vendor/yaml-cpp/include)
add_executable(yandere yandere.cpp)
set_property(TARGET yandere PROPERTY CXX_STANDARD 17)
set_property(TARGET yandere PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(yandere yaml-cpp mastodonpp)
