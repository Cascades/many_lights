add_executable(test_application
	main.cpp
)

target_link_libraries(test_application PUBLIC
	ML::ML
	TestApplication::RenderingAlgorithms::Deferred
	TestApplication::RenderingAlgorithms::Forward
	TestApplication::RenderingAlgorithms::StochasticLightCuts
)

install(TARGETS test_application)