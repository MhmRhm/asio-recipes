enable_testing()

find_package(GTest CONFIG REQUIRED)

include(GoogleTest)
include(Boost)
include(Coverage)
include(Memcheck)

macro(AddGTests target)
	AddCoverage(${target})
	target_link_libraries(${target}
		PRIVATE GTest::gtest
		PRIVATE GTest::gtest_main
		PRIVATE GTest::gmock
		PRIVATE GTest::gmock_main
	)
	gtest_discover_tests(${target})
	AddMemcheck(${target})
endmacro()

macro(AddBoostTests target)
	AddCoverage(${target})
	target_link_libraries(${target} PRIVATE Boost::unit_test_framework)
	add_test(NAME "${target}" COMMAND ${target}) # All tests run as one
	AddMemcheck(${target})
endmacro()
