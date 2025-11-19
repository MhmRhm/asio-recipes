include(Boost)
include(Protobuf)

add_executable(bm_asio18_client main.cpp client.cpp)

target_compile_options(bm_asio18_client
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)

target_include_directories(bm_asio18_client
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(bm_asio18_client
	PRIVATE precompiled
	PRIVATE Boost::system
	PRIVATE Boost::coroutine2
	PRIVATE Boost::thread
	PRIVATE Boost::regex
	PRIVATE Boost::asio
	PRIVATE Boost::program_options
)
AddProtobuf(bm_asio18_client PROTOS
	WorkLoad.proto
)

Format(bm_asio18_client .)