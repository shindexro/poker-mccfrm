project (algorithms)

add_library(${PROJECT_NAME}
    src/kmeans.cpp
)

set_source_files_properties(
    src/kmeans.cpp
PROPERTIES COMPILE_OPTIONS "-g")

add_library(sub::algorithms ALIAS ${PROJECT_NAME})

find_package(TBB REQUIRED)

target_include_directories( ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/../../include
)

target_link_libraries(${PROJECT_NAME}
    sub::utils
    sub::abstraction
    TBB::tbb
)
