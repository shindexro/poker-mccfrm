project(test)

enable_testing()

include(GoogleTest)

fuzztest_setup_fuzzing_flags()

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
  PRIVATE
  GTest::gtest_main
  GTest::gmock_main
  sub::game
  sub::abstraction
  sub::tables
)

link_fuzztest(test)
gtest_discover_tests(test PROPERTIES DISCOVERY_TIMEOUT 60)
