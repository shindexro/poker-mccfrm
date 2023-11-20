project (abstraction)

add_library(${PROJECT_NAME}
    src/chance_state.cpp
    src/community_info.cpp
    src/global.cpp
    src/infoset.cpp
    src/play_state.cpp
    src/player_info.cpp
    src/state.cpp
    src/terminal_state.cpp
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
