project(test)

enable_testing()

include(GoogleTest)

add_executable(
  test
  card.cpp
  hand.cpp
  hand_strength.cpp
  deck.cpp
  hand_indexer.cpp
  chance_state.cpp
  evaluator.cpp
)

target_link_libraries(
  test
  PRIVATE
  GTest::gtest_main
  GTest::gmock_main
  sub::game
  sub::abstraction
  sub::tables
)

gtest_discover_tests(test PROPERTIES DISCOVERY_TIMEOUT 60)
