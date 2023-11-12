project (enums)

add_library(${PROJECT_NAME}
    src/action.cpp
    src/betting_round.cpp
    src/hand_ranking.cpp
)

add_library(sub::enums ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
