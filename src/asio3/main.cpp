#include <boost/asio.hpp>
#include <iostream>

int main() {
  boost::asio::io_context ioc{};
  boost::asio::ip::tcp protocol{boost::asio::ip::tcp::v4()};
  boost::asio::ip::tcp::socket socket{ioc};
  boost::system::error_code ec{};

  socket.open(protocol, ec);
  if (ec.value()) {
    std::cerr << std::format("ec.value = {}, ec.message = {}", ec.value(),
                             ec.message())
              << std::endl;
    return ec.value();
  }

  return 0;
}
