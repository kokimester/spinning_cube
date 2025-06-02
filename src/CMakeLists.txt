# Executable name can be variable
add_executable(main main.c)

# Optionally link any libraries that are needed by the binary

target_link_libraries(main PRIVATE obj_parser ncurses m)

get_target_property(MAIN_CFLAGS main COMPILE_OPTIONS)
# also see: COMPILE_DEFINITIONS INCLUDE_DIRECTORIES
message("-- Target compiler flags are: ${MAIN_CFLAGS}")

add_custom_command(TARGET main POST_BUILD
COMMAND echo built with the flags: ${MAIN_CFLAGS})

