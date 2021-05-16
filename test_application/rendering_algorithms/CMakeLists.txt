add_library(rendering_algorithms STATIC
	forward_renderer.cpp
	deferred_renderer.cpp
)

target_include_directories(rendering_algorithms PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rendering_algorithms PUBLIC
	ML::ML
)

add_library(TestApplication::RenderingAlgorithms ALIAS rendering_algorithms)