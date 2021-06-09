add_library(many_lights STATIC
	window.cpp
    )

target_include_directories(many_lights PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(many_lights PUBLIC
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

add_library(ML::ML ALIAS many_lights)

set_target_properties(many_lights PROPERTIES PUBLIC_HEADER "include/many_lights/many_lights.h")
INSTALL(TARGETS many_lights 
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/include/many_lights
)
