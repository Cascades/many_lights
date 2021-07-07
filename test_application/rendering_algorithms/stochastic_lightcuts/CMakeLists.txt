add_subdirectory(PBT)
add_subdirectory(LightCut)

add_library(rendering_algorithms_sochastic_lightcuts INTERFACE)

target_include_directories(rendering_algorithms_sochastic_lightcuts INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rendering_algorithms_sochastic_lightcuts INTERFACE
	ML::ML
	TestApplication::RenderingAlgorithms::StochasticLightCuts::PBT
	TestApplication::RenderingAlgorithms::StochasticLightCuts::LightCut
)

add_library(TestApplication::RenderingAlgorithms::StochasticLightCuts ALIAS rendering_algorithms_sochastic_lightcuts)