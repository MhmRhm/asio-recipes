include(Boost)
include(Protobuf)

add_executable(asio13_client main.cpp)

target_compile_options(asio13_client
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)

target_include_directories(asio13_client
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(asio13_client
	PRIVATE precompiled
	PRIVATE Boost::system
	PRIVATE Boost::coroutine2
	PRIVATE Boost::thread
	PRIVATE Boost::regex
	PRIVATE Boost::asio
	PRIVATE Boost::program_options
)
AddProtobuf(asio13_client PROTOS
	Person.proto
)

Format(asio13_client .)