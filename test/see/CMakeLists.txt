add_executable(boost_test_see
	boost_test_terminal_see.cpp
)

target_link_libraries(boost_test_see
	PRIVATE precompiled
	PRIVATE terminal_see_static
)

Format(boost_test_see .)
AddBoostTests(boost_test_see)
EnableCoverage(terminal_see_static)
