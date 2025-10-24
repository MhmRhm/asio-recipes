include(Boost)
include(Protobuf)

add_executable(asio12_send_receive main.cpp)

target_compile_options(asio12_send_receive
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)

target_link_libraries(asio12_send_receive
	PRIVATE precompiled
	PRIVATE Boost::system
	PRIVATE Boost::coroutine2
	PRIVATE Boost::thread
	PRIVATE Boost::regex
	PRIVATE Boost::asio
	PRIVATE Boost::program_options
)
AddProtobuf(asio12_send_receive PROTOS
	Person.proto
)

Format(asio12_send_receive .)