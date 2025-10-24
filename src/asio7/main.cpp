#include <boost/asio.hpp>
#include <iostream>

int main() {
  boost::system::error_code ec{};
  auto protocol{boost::asio::ip::tcp::v4()};
  boost::asio::io_context ioc{};
  boost::asio::ip::tcp::socket socket{ioc};
  boost::asio::ip::tcp::endpoint endpoint{
      boost::asio::ip::make_address("127.0.0.1"), 8172};

  std::cout << "Openning client socket..." << std::endl;
  socket.open(protocol, ec);
  if (ec.value()) {
    goto err_out;
  }

  std::cout << "Connecting client socket..." << std::endl;
  socket.connect(endpoint, ec);
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
