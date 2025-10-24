#include <boost/asio.hpp>
#include <iostream>

int main() {
  std::array<uint8_t, 1024 * 4> data{};
  auto buffer{boost::asio::buffer(data)};
  return 0;
}
