include(FetchContent)

FetchContent_Declare(
	Protobuf
	GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
	GIT_TAG v32.1
	GIT_SHALLOW 1
)

set(protobuf_BUILD_TESTS OFF)

FetchContent_MakeAvailable(Protobuf)
include(${protobuf_SOURCE_DIR}/cmake/protobuf-generate.cmake)

# function(AddProtobuf target)
# 	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${target}_pb")
# 
# 	target_link_libraries(${target} PRIVATE protobuf::libprotobuf)
# 	target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
# 
# 	protobuf_generate(TARGET ${target}
# 		PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${target}_pb"
# 	)
# endfunction()

function(AddProtobuf target)
	cmake_parse_arguments(ARG "" "" "PROTOS" ${ARGN})

	set(PROTO_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${target}_pb")
	file(MAKE_DIRECTORY "${PROTO_OUT_DIR}")

	# Create a library target for generated code
	set(PROTO_LIB "${target}_proto")
	add_library(${PROTO_LIB} STATIC ${ARG_PROTOS})
	target_compile_options(${PROTO_LIB}
		PRIVATE ${DEFAULT_CXX_COMPILE_FLAGS}
		PRIVATE ${DEFAULT_CXX_OPTIMIZE_FLAG}
	)

	# Generate protobuf sources
	protobuf_generate(
		TARGET ${PROTO_LIB}
		PROTOC_OUT_DIR "${PROTO_OUT_DIR}"
	)

	# Include generated headers and link protobuf runtime
	target_include_directories(${PROTO_LIB} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
	target_link_libraries(${PROTO_LIB} PUBLIC protobuf::libprotobuf)

	# Link the main target against the proto lib
	target_link_libraries(${target} PRIVATE ${PROTO_LIB})
endfunction()
