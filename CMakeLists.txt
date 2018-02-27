cmake_minimum_required (VERSION 2.8)

# Handle recommended policies as of CMake 3.10
foreach (pol
	CMP0056
	CMP0065
	CMP0066
)
	if (POLICY ${pol})
		cmake_policy (SET ${pol} NEW)
	endif ()
endforeach ()

project (metrognome C)

find_package (PkgConfig REQUIRED)
pkg_check_modules (GTK REQUIRED gtk+-3.0)
pkg_check_modules (GSTREAMER REQUIRED gstreamer-1.0)

add_executable (metrognome main.c metronome.c)
target_include_directories (metrognome
	PRIVATE ${GTK_INCLUDE_DIRS}
	PRIVATE ${GSTREAMER_INCLUDE_DIRS}
	PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries (metrognome
	${GTK_LIBRARIES}
	${GSTREAMER_LIBRARIES}
)

set (VERSION "0.1.0")
set (METROGNOME_DATADIR "${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}")
configure_file(config.h.meson config.h @ONLY)

install (TARGETS metrognome
	RUNTIME DESTINATION bin
)
install (FILES metrognome.ui
	DESTINATION ${METROGNOME_DATADIR}
)
