set(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(ASSETS_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/assets)

set(ASSETS_FROM_MAIN "../assets/")

add_custom_target(assets_targ ALL
	COMMENT "Copying assets dir"
	COMMAND ${CMAKE_COMMAND} -E make_directory ${ASSETS_INSTALL_DIR} ${ASSETS_BUILD_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_DIR} ${ASSETS_INSTALL_DIR}
)
