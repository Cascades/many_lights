add_library(rendering STATIC
	mesh.cpp
	shader.cpp
	renderer.cpp
 "include/many_lights/model.h" "model.cpp" "buffer.cpp" "include/many_lights/buffer.h" "buffer.cpp" "include/many_lights/buffer.h" "include/many_lights/mesh.h" "include/many_lights/model.h" "include/many_lights/renderer.h" "include/many_lights/shader.h" "mesh.cpp" "model.cpp" "renderer.cpp" "shader.cpp" "include/many_lights/many_lights_algorithm.h")

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
	ML::Scene
	ML::Camera
)

add_library(ML::Rendering ALIAS rendering)