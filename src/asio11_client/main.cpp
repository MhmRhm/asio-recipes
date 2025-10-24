#include <boost/asio.hpp>
#include <iostream>

int main() {
  std::string data{"Hello\nWorld!"};
  auto buff{boost::asio::buffer(data)};

  boost::system::error_code ec{};
  auto address{boost::asio::ip::make_address("127.0.0.1")};
  boost::asio::ip::tcp::endpoint endpoint{address, 8172};
  auto protocol{boost::asio::ip::tcp::v4()};
  boost::asio::io_context ioc{};
  boost::asio::ip::tcp::socket socket{ioc};

  socket.open(protocol, ec);
  if (ec.value()) {
    goto err_out;
  }

  socket.connect(endpoint, ec);
  if (ec.value()) {
    goto err_out;
  }

  boost::asio::write(socket, boost::asio::buffer(data), ec);
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
