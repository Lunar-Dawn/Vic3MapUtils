find_package(Git)

if (GIT_EXECUTABLE)
	execute_process(
			COMMAND ${GIT_EXECUTABLE} describe --tags --dirty
			OUTPUT_VARIABLE PROGRAM_VERSION
			OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif ()

if (PROGRAM_VERSION STREQUAL "")
	set(PROGRAM_VERSION 0.0.0-unknown)
	message(WARNING "No version tag found")
endif ()

configure_file(${SRC} ${DST} @ONLY)
