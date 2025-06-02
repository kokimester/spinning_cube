# Obj_parser library
add_library(
    obj_parser
    obj_parser/obj_parser.c
    obj_parser/list.c
    obj_parser/string_extra.c
)
#target_compile_options(obj_parser PRIVATE -Wno-unused-function)
target_include_directories(
    obj_parser PUBLIC obj_parser
)
