project(test)

enable_testing()

add_executable(
  test
  card.cpp
  hand.cpp
)

target_link_libraries(
  test
  GTest::gtest_main
  GTest::gmock_main
  sub::game
)

include(GoogleTest)
gtest_discover_tests(test)
