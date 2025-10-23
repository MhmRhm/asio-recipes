include(Boost)

enable_testing()

add_executable(boost_test_see
	boost_test_terminal_see.cpp
)

target_link_libraries(boost_test_see
	PRIVATE precompiled
	PRIVATE Boost::unit_test_framework
	PRIVATE terminal_see_static
)

# All tests seen as one
add_test(NAME BoostTestsSee COMMAND boost_test_see)

Format(boost_test_see .)
