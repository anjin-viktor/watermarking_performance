project(watermark)
cmake_minimum_required(VERSION 3.0)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME OFF)

add_subdirectory(src)
add_subdirectory(tests)
enable_testing()
