add_library(ui STATIC
	ui.cpp
)

target_include_directories(ui PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ui
	glfw
	glad
	ML::Scene
)

add_library(ML::UI ALIAS ui)