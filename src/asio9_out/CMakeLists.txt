include(Boost)

add_executable(asio9_out main.cpp)

target_compile_options(asio9_out
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)

target_link_libraries(asio9_out
	PRIVATE precompiled
	PRIVATE Boost::system
	PRIVATE Boost::coroutine2
	PRIVATE Boost::thread
	PRIVATE Boost::regex
	PRIVATE Boost::asio
)

Format(asio9_out .)