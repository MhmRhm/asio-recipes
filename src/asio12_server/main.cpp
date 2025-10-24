#include <boost/asio.hpp>
#include <iostream>

int main() {
  char data[12];
  const size_t MSG_SIZE{12};

  boost::system::error_code ec{};
  boost::asio::io_context ioc{};
  auto protocol{boost::asio::ip::tcp::v4()};
  boost::asio::ip::tcp::socket socket{ioc};
  boost::asio::ip::tcp::acceptor acceptor{ioc};
  boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::address_v4::any(),
                                          8172};

  acceptor.open(protocol, ec);
  if (ec.value()) {
    goto err_out;
  }

  acceptor.bind(endpoint, ec);
  if (ec.value()) {
    goto err_out;
  }

  acceptor.listen(4096, ec);
  if (ec.value()) {
    goto err_out;
  }

  acceptor.accept(socket, ec);
  if (ec.value()) {
    goto err_out;
  }

  // while (bytes_read != MSG_SIZE) {
  //   bytes_read += socket.read_some(
  //       boost::asio::buffer(data + bytes_read, MSG_SIZE - bytes_read), ec);
  //   if (ec.value()) {
  //     goto err_out;
  //   }
  // }

  boost::asio::read(socket, boost::asio::buffer(data, MSG_SIZE), ec);
  if (ec.value()) {
    goto err_out;
  }

  std::cout << std::format("Received message is [{}]",
                           std::string{&data[0], MSG_SIZE})
            << std::endl;
  return 0;

err_out:
  std::cerr << std::format("ec.value = {}, ec.message = {}", ec.value(),
                           ec.message())
            << std::endl;
  return ec.value();
}
