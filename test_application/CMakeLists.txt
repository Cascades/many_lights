list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/third_party/FindTBB)

if(UNIX)
	find_package(TBB REQUIRED)
endif()

add_subdirectory(test_application)
add_subdirectory(rendering_algorithms)