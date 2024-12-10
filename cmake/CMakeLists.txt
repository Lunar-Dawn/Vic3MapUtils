add_custom_target(version
		${CMAKE_COMMAND}
		-D SRC=${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in
		-D DST=${CMAKE_SOURCE_DIR}/src/version.hpp
		-P ${CMAKE_CURRENT_SOURCE_DIR}/version.cmake
)
