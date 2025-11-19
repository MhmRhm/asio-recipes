include(Boost)
include(Protobuf)

add_executable(bm_asio_server main.cpp server.cpp client.cpp)

target_compile_options(bm_asio_server
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)

target_include_directories(bm_asio_server
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(bm_asio_server
	PRIVATE precompiled
	PRIVATE Boost::system
	PRIVATE Boost::coroutine2
	PRIVATE Boost::thread
	PRIVATE Boost::regex
	PRIVATE Boost::asio
	PRIVATE Boost::program_options
)
AddProtobuf(bm_asio_server PROTOS
	WorkLoad.proto
)

Format(bm_asio_server .)