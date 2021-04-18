add_library(rendering STATIC
	mesh.cpp
	shader.cpp
	renderer.cpp
 "include/many_lights/model.h" "model.cpp")

target_include_directories(rendering PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rendering
	glfw
	glad
	glm::glm
	stb::stb
	assimp::assimp
	ML::Exceptions
)

add_library(ML::Rendering ALIAS rendering)