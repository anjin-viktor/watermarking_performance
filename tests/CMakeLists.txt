project(tests)

find_package(Boost COMPONENTS system unit_test_framework REQUIRED)

include_directories(
  ${Boost_INCLUDE_DIR}
)

set(HEADERS
  ../third_party/csv2/csv2.hpp
)

set(TEST_SOURCES
  VideoFrame.cpp
  WatermarkReference.cpp
  Detector.cpp
  Performance.cpp
)

add_executable(tests ${TEST_SOURCES} ${HEADERS}) 
target_link_libraries(tests  watermark ${Boost_LIBRARIES})
target_include_directories(tests PUBLIC ../third_party/csv2)
add_dependencies(tests watermark)

set_target_properties(tests PROPERTIES FOLDER Tests)
add_test(NAME tests COMMAND tests)

enable_testing()
