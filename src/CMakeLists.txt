set(SOURCES
	VideoFrame.cpp
#	EBlindDLC.cpp
#	Watermark.cpp
	WatermarkReference.cpp
	Detector.cpp
)

set(HEADERS
	VideoFrame.h
#	EBlindDLC.h
#	Watermark.h
	WatermarkReference.h
	Detector.h
	../third_party/ThreadPool/ThreadPool.h
)

add_library(watermark ${SOURCES} ${HEADERS}) 

find_package(OpenCV REQUIRED)
target_include_directories(watermark PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS} ../third_party/ThreadPool)
target_link_libraries(watermark PUBLIC ${OpenCV_LIBS})