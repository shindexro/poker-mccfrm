project (game)

add_library(${PROJECT_NAME}
    src/card.cpp
    src/deck.cpp
    src/hand.cpp
)
add_library(sub::game ALIAS ${PROJECT_NAME})

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    sub::enums
)
