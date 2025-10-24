#include <boost/asio.hpp>
#include <iostream>

int main() {
  boost::system::error_code ec{};
  boost::asio::io_context ioc{};
  auto address{boost::asio::ip::address_v4::any()};
  auto porotocol{boost::asio::ip::tcp::v4()};
  boost::asio::ip::tcp::endpoint endpoint{address, 8172};
  boost::asio::ip::tcp::acceptor acceptor{ioc};

  acceptor.open(porotocol, ec);
  if (ec.value()) {
    goto err_out;
  }

  acceptor.bind(endpoint, ec);
  if (ec.value()) {
    goto err_out;
  }

  return 0;

err_out:
  std::cerr << std::format("ec.value = {}, ec.message = {}", ec.value(),
                           ec.message())
            << std::endl;
  return ec.value();
}
