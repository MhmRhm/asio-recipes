#include <boost/asio.hpp>
#include <iostream>

int main() {
  boost::asio::io_context ioc{};
  boost::asio::ip::tcp::resolver resolver{ioc};
  boost::system::error_code ec{};

  auto results{resolver.resolve("google.com", "80", ec)};
  if (ec.value()) {
    std::cerr << std::format("ec.value = {}, ec.message = {}", ec.value(),
                             ec.message())
              << std::endl;
    return ec.value();
  }

  for (auto &&res : results) {
    std::cout << std::format("{}:{}", res.endpoint().address().to_string(),
                             res.endpoint().port())
              << std::endl;
  }
}
