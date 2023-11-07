project(test)

enable_testing()

add_executable(
  test
  test.cpp
  card.cpp
)

target_link_libraries(
  test
  GTest::gtest_main
  sub::game
)

include(GoogleTest)
gtest_discover_tests(test)
