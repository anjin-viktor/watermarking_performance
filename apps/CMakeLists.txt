find_package(Boost COMPONENTS program_options REQUIRED)

include_directories(
  ${Boost_INCLUDE_DIR}
)

add_executable(eblind_dlc eblind_dlc.cpp)
target_link_libraries(eblind_dlc watermark ${Boost_LIBRARIES})

add_executable(eblind_dlc_dctdniz_robust eblind_dlc_dctdniz_robust.cpp)
target_link_libraries(eblind_dlc_dctdniz_robust watermark ${Boost_LIBRARIES})

add_executable(color_to_gray color_to_gray.cpp)
target_link_libraries(color_to_gray watermark ${Boost_LIBRARIES})
