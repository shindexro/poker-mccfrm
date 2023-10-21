project (utils)

add_library(${PROJECT_NAME}
    src/utils.cpp
    src/random.cpp
)
add_library(sub::utils ALIAS ${PROJECT_NAME})

target_include_directories( ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# target_link_libraries(${PROJECT_NAME}
#     sub::enums
# )
