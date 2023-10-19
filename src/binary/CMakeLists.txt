
project(src)

add_executable(${PROJECT_NAME} main.cpp main.h)

set_source_files_properties(main.cpp PROPERTIES COMPILE_OPTIONS "-g")

target_link_libraries(${PROJECT_NAME}
    sub::game
    sub::abstraction
    sub::tables
)
