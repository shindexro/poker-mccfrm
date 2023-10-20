project (utils)

add_library(${PROJECT_NAME}
    src/ehs_table.cpp
    src/hand_indexer.cpp
    src/evaluator.cpp
    ochs_table.cpp
)
add_library(sub::utils ALIAS ${PROJECT_NAME})

target_include_directories( ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    sub::enums
)
