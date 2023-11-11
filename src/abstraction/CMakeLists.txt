project (abstraction)

add_library(${PROJECT_NAME}
    src/global.cpp
    src/infoset.cpp
    src/state.cpp
    src/community_info.cpp
    src/player_info.cpp
)

add_library(sub::abstraction ALIAS ${PROJECT_NAME})

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    sub::tables
    sub::enums
    sub::game
)
