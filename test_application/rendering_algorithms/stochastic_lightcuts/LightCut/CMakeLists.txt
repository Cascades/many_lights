add_library(lightcut INTERFACE)

target_include_directories(lightcut INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(lightcut INTERFACE
	ML::ML
	TestApplication::RenderingAlgorithms::StochasticLightCuts::PBT
)

add_library(TestApplication::RenderingAlgorithms::StochasticLightCuts::LightCut ALIAS lightcut)