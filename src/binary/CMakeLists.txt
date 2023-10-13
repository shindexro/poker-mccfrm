
project(src)

add_executable(${PROJECT_NAME} main.cpp main.h)

target_link_libraries(${PROJECT_NAME}
    sub::game
)
