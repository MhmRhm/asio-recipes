#include <boost/asio.hpp>
#include <iostream>

int main() {
  std::string data{"Hello\nWorld!"};

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

  // while (bytes_written != data.size()) {
  //   bytes_written +=
  //       socket.write_some(boost::asio::buffer(data.c_str() + bytes_written,
  //                                             data.size() - bytes_written),
  //                         ec);
  //   if (ec.value()) {
  //     goto err_out;
  //   }
  // }

  boost::asio::write(socket, boost::asio::buffer(data), ec);
  return 0;

err_out:
  std::cerr << std::format("ec.value = {}, ec.message = {}", ec.value(),
                           ec.message())
            << std::endl;
  return ec.value();
}
