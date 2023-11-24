project(test)

enable_testing()

add_executable(
  test
  card.cpp
  hand.cpp
  hand_strength.cpp
  deck.cpp
  hand_indexer.cpp
  state.cpp
  evaluator.cpp
)

target_link_libraries(
  test
  GTest::gtest_main
  GTest::gmock_main
  sub::game
  sub::abstraction
  sub::tables
)

include(GoogleTest)
gtest_discover_tests(test PROPERTIES DISCOVERY_TIMEOUT 60)
