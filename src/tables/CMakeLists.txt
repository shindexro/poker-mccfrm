project (tables)

add_library(${PROJECT_NAME}
    src/ehs_table.cpp
    src/hand_indexer.cpp
    src/evaluator.cpp
)
add_library(sub::tables ALIAS ${PROJECT_NAME})

find_package(Boost 1.83.0 EXACT REQUIRED COMPONENTS
    serialization
)

target_include_directories( ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    sub::enums
    sub::game
    Boost::boost
)
