project (METROGNOME)
cmake_minimum_required (VERSION 2.8)

find_package (PkgConfig REQUIRED)
pkg_check_modules (GTK3 REQUIRED gtk+-3.0)

include_directories (${GTK3_INCLUDE_DIRS})
link_directories (${GTK3_LIBRARY_DIRS})
add_executable (metrognome main.c metronome.c)
add_definitions (${GTK3_CFLAGS_OTHER} -Wall -Wextra -ggdb3)
target_link_libraries (metrognome ${GTK3_LIBRARIES})

