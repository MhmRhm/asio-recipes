add_library(see_app_static STATIC
	see_app.cpp
)

target_include_directories(see_app_static
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(see_app_static
	PRIVATE precompiled
	PUBLIC libsee_static
)

AddBuildInfo(see_app_static)
CleanCoverage(see_app_static)
Format(see_app_static .)
AddCppcheck(see_app_static)
Doxygen(see_app_static src/see)

add_executable(see main.cpp)

target_compile_options(see
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)

target_link_libraries(see
	PRIVATE precompiled
	PRIVATE see_app_static
)
