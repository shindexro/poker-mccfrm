project (abstraction)

add_library(${PROJECT_NAME}
    src/global.cpp
)
add_library(sub::abstraction ALIAS ${PROJECT_NAME})

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    sub::abstraction
)
