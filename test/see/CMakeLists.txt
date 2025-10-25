add_executable(boost_test_see_app
	boost_test_see_app.cpp
)

target_link_libraries(boost_test_see_app
	PRIVATE precompiled
	PRIVATE see_app_static
)

# All tests appear as one
add_test(NAME BoostTest COMMAND boost_test_see_app)

Format(boost_test_see_app .)
AddBoostTests(boost_test_see_app)
EnableCoverage(see_app_static)
