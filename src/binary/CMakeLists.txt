
project(src)

add_executable(${PROJECT_NAME} main.cpp include/binary/main.h)

target_link_libraries(${PROJECT_NAME}
    sub::abstraction
    sub::tables
    sub::algorithms
    sub::utils
    sub::game
)

target_include_directories( ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/../../include
)
