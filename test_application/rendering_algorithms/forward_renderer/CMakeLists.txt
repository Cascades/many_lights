add_library(rendering_algorithms_for STATIC
	forward_renderer.cpp
)

target_include_directories(rendering_algorithms_for PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rendering_algorithms_for PUBLIC
	ML::ML
)

add_library(TestApplication::RenderingAlgorithms::Forward ALIAS rendering_algorithms_for)