#include <boost/asio.hpp>
#include <iostream>

int main() {
  boost::system::error_code ec{};
  boost::asio::io_context ioc{};
  auto protocol{boost::asio::ip::tcp::v4()};
  boost::asio::ip::tcp::socket socket{ioc};
  boost::asio::ip::tcp::acceptor acceptor{ioc};
  boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::address_v4::any(),
                                          8172};

  std::cout << "Openning server..." << std::endl;
  acceptor.open(protocol, ec);
  if (ec.value()) {
    goto err_out;
  }

  std::cout << "Binding server..." << std::endl;
  acceptor.bind(endpoint, ec);
  if (ec.value()) {
    goto err_out;
  }

  std::cout << "Start listening..." << std::endl;
  acceptor.listen(30, ec);
  if (ec.value()) {
    goto err_out;
  }

  std::cout << "Start accepting..." << std::endl;
  acceptor.accept(socket, ec);
  if (ec.value()) {
    goto err_out;
  }

  std::cout << "Connection established." << std::endl;
  std::cout << std::format("{}:{}",
                           socket.local_endpoint().address().to_string(),
                           socket.local_endpoint().port())
            << std::endl;
  return 0;

err_out:
  std::cerr << std::format("ec.value = {}, ec.message = {}", ec.value(),
                           ec.message())
            << std::endl;
  return ec.value();
}
