include(Protobuf)

find_package(Qt6 REQUIRED COMPONENTS Core Network Concurrent StateMachine)
qt_standard_project_setup()

qt_add_executable(bm_qt19_server main.cpp)
target_sources(bm_qt19_server PRIVATE
    server.cpp
	client.cpp
	include/bm_qt19_server/server.h
	include/bm_qt19_server/client.h
)

target_compile_options(bm_qt19_server
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)

target_include_directories(bm_qt19_server
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(bm_qt19_server
	PRIVATE precompiled
	PRIVATE Qt6::Core
	PRIVATE Qt6::Network
	PRIVATE Qt6::Concurrent
	PRIVATE Qt6::StateMachine
	PRIVATE protobuf::libprotobuf
)
AddProtobuf(bm_qt19_server PROTOS
	WorkLoad.proto
)

Format(bm_qt19_server .)