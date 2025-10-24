#include <boost/asio.hpp>
#include <iostream>

int main() {
  auto ip_address{boost::asio::ip::address_v4::any()};
  unsigned short port_num{3333};

  boost::asio::ip::tcp::endpoint ep{ip_address, port_num};

  return 0;
}
