add_library(scene STATIC
	scene.cpp
 "include/many_lights/scene_lights.h")

target_include_directories(scene PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(scene
	glfw
	glad
	glm::glm
	stb::stb
	assimp::assimp
	ML::Exceptions
	ML::Rendering
	ML::Colors
)

add_library(ML::Scene ALIAS scene)