add_library(rendering STATIC
	mesh.cpp
	shader.cpp
	renderer.cpp
    model.cpp
)

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

#HACK

set_target_properties(rendering PROPERTIES PUBLIC_HEADER "include/many_lights/many_lights_algorithm.h;include/many_lights/shader.h")
INSTALL(FILES
	include/many_lights/many_lights_algorithm.h
	include/many_lights/shader.h
	DESTINATION
	${CMAKE_INSTALL_PREFIX}/include/many_lights
)
