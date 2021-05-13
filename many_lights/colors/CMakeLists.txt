add_library(colors STATIC
	colors.cpp
)

target_include_directories(colors PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(colors
	glm::glm
)

add_library(ML::Colors ALIAS colors)