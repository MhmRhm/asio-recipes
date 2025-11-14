include(Protobuf)

find_package(Qt6 REQUIRED COMPONENTS Core Network Concurrent StateMachine)
qt_standard_project_setup()

qt_add_executable(qt20_server main.cpp)
target_sources(qt20_server PRIVATE
    server.cpp
	client.cpp
	include/qt20_server/server.h
	include/qt20_server/client.h
)

target_compile_options(qt20_server
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)

target_include_directories(qt20_server
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(qt20_server
	PRIVATE precompiled
	PRIVATE Qt6::Core
	PRIVATE Qt6::Network
	PRIVATE Qt6::Concurrent
	PRIVATE Qt6::StateMachine
	PRIVATE protobuf::libprotobuf
)
AddProtobuf(qt20_server PROTOS
	WorkLoad.proto
)

Format(qt20_server .)