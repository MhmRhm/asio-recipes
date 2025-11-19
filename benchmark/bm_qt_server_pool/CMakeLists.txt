include(Protobuf)

find_package(Qt6 REQUIRED COMPONENTS Core Network Concurrent StateMachine)
qt_standard_project_setup()

qt_add_executable(bm_qt_server_pool main.cpp)
target_sources(bm_qt_server_pool PRIVATE
    server.cpp
	client.cpp
	include/bm_qt_server_pool/server.h
	include/bm_qt_server_pool/client.h
)

target_compile_options(bm_qt_server_pool
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)

target_include_directories(bm_qt_server_pool
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(bm_qt_server_pool
	PRIVATE precompiled
	PRIVATE Qt6::Core
	PRIVATE Qt6::Network
	PRIVATE Qt6::Concurrent
	PRIVATE Qt6::StateMachine
	PRIVATE protobuf::libprotobuf
)
AddProtobuf(bm_qt_server_pool PROTOS
	WorkLoad.proto
)

Format(bm_qt_server_pool .)