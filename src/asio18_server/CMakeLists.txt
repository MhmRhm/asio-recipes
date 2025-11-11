include(Boost)
include(Protobuf)

add_executable(asio18_server main.cpp server.cpp client.cpp)

target_compile_options(asio18_server
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)

target_include_directories(asio18_server
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(asio18_server
	PRIVATE precompiled
	PRIVATE Boost::system
	PRIVATE Boost::coroutine2
	PRIVATE Boost::thread
	PRIVATE Boost::regex
	PRIVATE Boost::asio
	PRIVATE Boost::program_options
)
AddProtobuf(asio18_server PROTOS
	WorkLoad.proto
)

Format(asio18_server .)