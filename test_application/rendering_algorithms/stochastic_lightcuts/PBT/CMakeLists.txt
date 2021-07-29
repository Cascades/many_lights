add_library(rendering_algorithms_PBT INTERFACE)

target_include_directories(rendering_algorithms_PBT INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rendering_algorithms_PBT INTERFACE
	ML::ML
	$<$<PLATFORM_ID:Linux>:tbb>
)

add_library(TestApplication::RenderingAlgorithms::StochasticLightCuts::PBT ALIAS rendering_algorithms_PBT)