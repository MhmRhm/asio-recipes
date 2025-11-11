include(Protobuf)

find_package(Qt6 REQUIRED COMPONENTS Core Network Concurrent StateMachine)
qt_standard_project_setup()

qt_add_executable(qt19_client main.cpp)
target_sources(qt19_client PRIVATE
    client.cpp
	include/qt19_client/client.h
)

target_compile_options(qt19_client
	PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
	PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
)

target_include_directories(qt19_client
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(qt19_client
	PRIVATE precompiled
	PRIVATE Qt6::Core
	PRIVATE Qt6::Network
	PRIVATE Qt6::Concurrent
	PRIVATE Qt6::StateMachine
	PRIVATE protobuf::libprotobuf
)
AddProtobuf(qt19_client PROTOS
	WorkLoad.proto
)

Format(qt19_client .)