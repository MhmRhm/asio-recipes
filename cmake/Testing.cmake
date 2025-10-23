enable_testing()

find_package(GTest CONFIG REQUIRED)

include(GoogleTest)
include(Coverage)
include(Memcheck)

macro(AddTests target)
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
