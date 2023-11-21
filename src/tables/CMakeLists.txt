project (tables)

add_library(${PROJECT_NAME}
    # src/ehs_table.cpp - deprecated
    src/hand_indexer.cpp
    src/evaluator.cpp
    src/ochs_table.cpp
    src/emd_table.cpp
)

add_library(sub::tables ALIAS ${PROJECT_NAME})

find_package(Boost 1.83.0 EXACT REQUIRED COMPONENTS
    serialization
)
find_package(TBB REQUIRED)

target_include_directories( ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/../../include
    ${Boost_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    sub::enums
    sub::game
    sub::utils
    sub::abstraction
    sub::algorithms
    Boost::boost
    TBB::tbb
    ${Boost_LIBRARIES}
)
