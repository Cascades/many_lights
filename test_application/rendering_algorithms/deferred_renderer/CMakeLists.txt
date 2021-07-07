add_library(rendering_algorithms_def STATIC
	deferred_renderer.cpp
)

target_include_directories(rendering_algorithms_def PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rendering_algorithms_def PUBLIC
	ML::ML
)

add_library(TestApplication::RenderingAlgorithms::Deferred ALIAS rendering_algorithms_def)