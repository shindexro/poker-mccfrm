project (algorithms)

add_library(${PROJECT_NAME}
    src/kmeans.cpp
    src/trainer.cpp
    src/player.cpp
    src/ai_player.cpp
    src/interactive_player.cpp
    src/game.cpp
)

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
