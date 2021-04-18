add_executable (many_lights
	many_lights.cpp
	window.cpp
)

target_include_directories(many_lights PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(many_lights
	glfw
	glad
	glm::glm
	assimp::assimp
	ML::Rendering
	ML::Camera
)

install(TARGETS many_lights)
