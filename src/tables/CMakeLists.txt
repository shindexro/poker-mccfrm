project (table)

add_library(${PROJECT_NAME}
    src/ehs_table
)
add_library(sub::table ALIAS ${PROJECT_NAME})

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    sub::enums
)
