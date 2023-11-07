project(test)

enable_testing()

add_executable(
  test
  card.cpp
  hand.cpp
  hand_strength.cpp
  deck.cpp
)

target_link_libraries(
  test
  GTest::gtest_main
  GTest::gmock_main
  sub::game
  sub::abstraction
)

include(GoogleTest)
gtest_discover_tests(test)
