include(Boost)
include(Protobuf)

add_executable(asio13_server main.cpp)

target_compile_options(asio13_server
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)

target_include_directories(asio13_server
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(asio13_server
	PRIVATE precompiled
	PRIVATE Boost::system
	PRIVATE Boost::coroutine2
	PRIVATE Boost::thread
	PRIVATE Boost::regex
	PRIVATE Boost::asio
	PRIVATE Boost::program_options
)
AddProtobuf(asio13_server PROTOS
	Person.proto
)

Format(asio13_server .)