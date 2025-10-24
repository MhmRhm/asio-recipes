#include <boost/asio.hpp>
#include <iostream>

int main() {
  std::string raw_ip_address{"127.0.0.1"};
  unsigned short port_num{3333};
  boost::system::error_code ec{};

  auto ip_address{boost::asio::ip::make_address(raw_ip_address, ec)};
  if (ec.value()) {
    std::cerr << std::format("ec.value = {}, ec.message = {}", ec.value(),
                             ec.message())
              << std::endl;
    return ec.value();
  }

  boost::asio::ip::tcp::endpoint ep{ip_address, port_num};
  return 0;
}
