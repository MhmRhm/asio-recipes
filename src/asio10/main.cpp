#include <boost/asio.hpp>
#include <iostream>

int main() {
  boost::asio::streambuf buf{};

  std::ostream output{&buf};
  output << "Some\nText";

  std::istream input{&buf};
  std::string msg{};
  while (input >> msg) {
    std::cout << std::format("Received message is [{}]", msg) << std::endl;
  }

  input.clear();

  char txt[100];
  output << "Some\nMore\nText";
  input.read(&txt[0], 100);
  std::cout << std::format("Received message is [{}]", &txt[0]) << std::endl;

  return 0;
}
