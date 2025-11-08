#include <iostream>
#include <numeric>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

using namespace boost::asio;

int server() {
  std::cout << "Running server variant..." << std::endl;

  std::array<std::uint8_t, 1024 * 4> data1{};
  std::array<std::uint8_t, 1024 * 4> data2{};
  std::iota(std::begin(data1), std::end(data1), 0);
  std::iota(std::begin(data2), std::end(data2), 0);
  const_buffer buff1{data1.data(), data1.size()};
  const_buffer buff2{data2.data(), data2.size()};
  std::vector<const_buffer> sg_buffer{buff1, buff2};
  size_t len{sizeof(data1) + sizeof(data2)};

  io_context ioc{};
  auto protocol{ip::tcp::v4()};
  ip::tcp::socket socket{ioc};
  ip::tcp::acceptor acceptor{ioc};
  ip::tcp::endpoint endpoint{ip::address_v4::any(), 8172};

  try {
    acceptor.open(protocol);
    acceptor.bind(endpoint);
    acceptor.listen();
    acceptor.accept(socket);

    write(socket, buffer(&len, sizeof(len)));
    write(socket, sg_buffer);
  } catch (const boost::system::system_error &e) {
    std::cerr << "Error occured! Error code = " << e.code()
              << ". Message: " << e.what();
    return e.code().value();
  }

  return 0;
}

int client() {
  std::cout << "Running client variant..." << std::endl;

  size_t len{};
  std::array<std::uint8_t, 1024 * 2> data1{};
  std::array<std::uint8_t, 1024 * 2> data2{};
  std::array<std::uint8_t, 1024 * 2> data3{};
  std::array<std::uint8_t, 1024 * 2> data4{};
  mutable_buffer buff1{data1.data(), data1.size()};
  mutable_buffer buff2{data2.data(), data2.size()};
  mutable_buffer buff3{data3.data(), data3.size()};
  mutable_buffer buff4{data4.data(), data4.size()};
  std::vector<mutable_buffer> sg_buffer{buff1, buff2, buff3, buff4};

  io_context ioc{};
  ip::tcp::socket socket{ioc};
  ip::tcp::endpoint endpoint{ip::make_address("127.0.0.1"), 8172};

  try {
    socket.open(endpoint.protocol());
    socket.connect(endpoint);

    read(socket, buffer(&len, sizeof(len)));
    read(socket, sg_buffer, transfer_exactly(len));
  } catch (const boost::system::system_error &e) {
    std::cerr << "Error occured! Error code = " << e.code()
              << ". Message: " << e.what();
    return e.code().value();
  }

  std::cout << std::format("Received {} bytes.", len) << std::endl;

  return 0;
}

int main(int argc, char *argv[]) {
  boost::program_options::options_description desc("Allowed options");
  desc.add_options()("server", "Run as server")("client", "Run as client");

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);

  if (vm.count("server")) {
    return server();
  } else if (vm.count("client")) {
    return client();
  } else {
    std::cout << desc << std::endl;
    return 1;
  }
}
