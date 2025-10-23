#define BOOST_TEST_MODULE test_terminal_see
#include "see/terminal_see.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(test_suite_terminal_see)

BOOST_AUTO_TEST_CASE(test_terminal_see) {
  // given
  bool result{};

  // when
  result = terminalSee();

  // then
  BOOST_REQUIRE_EQUAL(result, true);
}

BOOST_AUTO_TEST_CASE(test_terminal_see_again) {
  // given
  bool result{};

  // when
  result = terminalSee();

  // then
  BOOST_REQUIRE_EQUAL(result, true);
}

BOOST_AUTO_TEST_SUITE_END()