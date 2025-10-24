include(Boost)

add_executable(asio8 main.cpp)

target_compile_options(asio8
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)

target_link_libraries(asio8
	PRIVATE precompiled
	PRIVATE Boost::system
	PRIVATE Boost::coroutine2
	PRIVATE Boost::thread
	PRIVATE Boost::regex
	PRIVATE Boost::asio
)

Format(asio8 .)