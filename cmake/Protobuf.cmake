find_package(protobuf CONFIG REQUIRED)

function(AddProtobuf target)
	cmake_parse_arguments(ARG "" "" "PROTOS" ${ARGN})

	set(PROTO_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${target}_pb")
	file(MAKE_DIRECTORY "${PROTO_OUT_DIR}")

	# Create a library target for generated code
	set(PROTO_LIB "${target}_proto")
	add_library(${PROTO_LIB} STATIC ${ARG_PROTOS})

	# Generate protobuf sources
	protobuf_generate(
		TARGET ${PROTO_LIB}
		PROTOC_OUT_DIR "${PROTO_OUT_DIR}"
	)

	# Include generated headers and link protobuf runtime
	target_include_directories(${PROTO_LIB} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
	target_link_libraries(${PROTO_LIB}
		PUBLIC protobuf::libprotoc
		PUBLIC protobuf::libprotobuf
		PUBLIC protobuf::libprotobuf-lite
	)

	# Link the main target against the proto lib
	target_link_libraries(${target} PRIVATE ${PROTO_LIB})
endfunction()
