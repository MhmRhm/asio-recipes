find_package(ftxui CONFIG REQUIRED)

macro(AddFTXUI target)
	target_link_libraries(${target}
		PRIVATE ftxui::dom
		PRIVATE ftxui::screen
		PRIVATE ftxui::component
	)
endmacro()
