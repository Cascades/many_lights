add_library(vpl_renderer INTERFACE)

target_include_directories(vpl_renderer INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(vpl_renderer INTERFACE
	ML::ML
)

add_library(TestApplication::RenderingAlgorithms::VPLDebug ALIAS vpl_renderer)