add_library(benchmarker STATIC
	benchmarker.cpp
)

target_include_directories(benchmarker PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(benchmarker
	glfw
	glad
	glm::glm
)

add_library(ML::BenchMarker ALIAS benchmarker)