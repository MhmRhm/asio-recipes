include(Boost)

add_executable(asio11_server main.cpp)

target_compile_options(asio11_server
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)

target_link_libraries(asio11_server
	PRIVATE precompiled
	PRIVATE Boost::system
	PRIVATE Boost::coroutine2
	PRIVATE Boost::thread
	PRIVATE Boost::regex
	PRIVATE Boost::asio
)

Format(asio11_server .)