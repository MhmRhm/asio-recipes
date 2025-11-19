include(Protobuf)

find_package(Qt6 REQUIRED COMPONENTS Core Network Concurrent StateMachine)
qt_standard_project_setup()

qt_add_executable(bm_qt_client main.cpp)
target_sources(bm_qt_client PRIVATE
    client.cpp
	include/bm_qt_client/client.h
)

target_compile_options(bm_qt_client
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)

target_include_directories(bm_qt_client
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(bm_qt_client
	PRIVATE precompiled
	PRIVATE Qt6::Core
	PRIVATE Qt6::Network
	PRIVATE Qt6::Concurrent
	PRIVATE Qt6::StateMachine
	PRIVATE protobuf::libprotobuf
)
AddProtobuf(bm_qt_client PROTOS
	WorkLoad.proto
)

Format(bm_qt_client .)