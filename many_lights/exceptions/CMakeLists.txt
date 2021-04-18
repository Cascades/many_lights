add_library(exceptions STATIC
	exceptions.cpp
)

target_include_directories(exceptions PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(exceptions
	glfw
	glad
)

add_library(ML::Exceptions ALIAS exceptions)