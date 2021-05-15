add_executable (many_lights
	many_lights.cpp
	window.cpp
 "include/many_lights/lights_algorithm.h")

target_include_directories(many_lights PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(many_lights
	glfw
	glad
	glm::glm
	imgui::imgui
	assimp::assimp
	ML::Rendering
	ML::Camera
	ML::BenchMarker
	ML::Scene
	ML::Colors
	ML::UI
)

target_compile_definitions(many_lights PUBLIC NOMINMAX)

install(TARGETS many_lights)
