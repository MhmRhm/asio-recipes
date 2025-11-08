include(Boost)
include(Protobuf)

find_package(OpenSSL REQUIRED)

add_executable(asio15_ssl main.cpp)

target_compile_options(asio15_ssl
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)

target_link_libraries(asio15_ssl
	PRIVATE precompiled
	PRIVATE Boost::system
	PRIVATE Boost::coroutine2
	PRIVATE Boost::thread
	PRIVATE Boost::regex
	PRIVATE Boost::asio
	PRIVATE Boost::program_options
	PRIVATE OpenSSL::SSL
	PRIVATE OpenSSL::Crypto
)
AddProtobuf(asio15_ssl PROTOS
	Person.proto
)

Format(asio15_ssl .)