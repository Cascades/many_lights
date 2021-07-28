add_executable(VPL_gen
	main.cpp
)

target_link_libraries(VPL_gen PUBLIC
	ML::ML
	nanort
)

target_compile_definitions(VPL_gen PUBLIC
	ML_CPU_READ
)

install(TARGETS VPL_gen)