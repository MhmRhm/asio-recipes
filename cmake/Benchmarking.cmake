find_package(benchmark CONFIG REQUIRED)

macro(AddBenchmarks target)
	target_link_libraries(${target}
		PRIVATE benchmark::benchmark
		PRIVATE benchmark::benchmark_main
	)
endmacro()
