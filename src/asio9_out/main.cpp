#include <boost/asio.hpp>
#include <iostream>
#include <numeric>

int main() {
  std::array<std::uint8_t, 1024 * 4> data{};
  std::iota(std::begin(data), std::end(data), 0);
  boost::asio::const_buffer buffer{data.data(), data.size()};
  return 0;
}
