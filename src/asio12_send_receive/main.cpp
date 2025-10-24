#include <asio12_send_receive_pb/Person.pb.h>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <google/protobuf/util/json_util.h>
#include <iostream>

int server() {
  std::cout << "Running server variant..." << std::endl;

  Person me{};
  size_t len{};
  boost::asio::streambuf data_buf{};
  std::ostream data_out{&data_buf};

  me.set_id(1000);
  me.set_name("Mohammad");
  me.set_email("rahimi.mhmmd@gmail.com");

  me.SerializeToOstream(&data_out);
  len = data_buf.size();

  using namespace boost::asio;
  io_context ioc{};
  auto protocol{ip::tcp::v4()};
  ip::tcp::socket socket{ioc};
  ip::tcp::acceptor acceptor{ioc};
  ip::tcp::endpoint endpoint{ip::address_v4::any(), 8172};

  std::cout << std::format("Before: {}", data_buf.size()) << std::endl;

  try {
    acceptor.open(protocol);
    acceptor.bind(endpoint);
    acceptor.listen();
    acceptor.accept(socket);

    write(socket, buffer(&len, sizeof(len)));
    write(socket, data_buf);
  } catch (const boost::system::system_error &e) {
    std::cerr << "Error occured! Error code = " << e.code()
              << ". Message: " << e.what();
    return e.code().value();
  }

  std::cout << std::format("After: {}", data_buf.size()) << std::endl;

  return 0;
}

int client() {
  std::cout << "Running client variant..." << std::endl;

  Person me{};
  size_t len{};
  boost::asio::streambuf data_buf{};
  std::istream data_in{&data_buf};

  using namespace boost::asio;
  io_context ioc{};
  ip::tcp::socket socket{ioc};
  ip::tcp::endpoint endpoint{ip::make_address("127.0.0.1"), 8172};

  try {
    socket.open(endpoint.protocol());
    socket.connect(endpoint);

    read(socket, buffer(&len, sizeof(len)));
    read(socket, data_buf, transfer_exactly(len));

    me.ParseFromIstream(&data_in);
  } catch (const boost::system::system_error &e) {
    std::cerr << "Error occured! Error code = " << e.code()
              << ". Message: " << e.what();
    return e.code().value();
  }

  using namespace google::protobuf::util;
  std::string json{};
  auto status{
      MessageToJsonString(me, &json, JsonPrintOptions{.add_whitespace = true})};
  if (status.ok()) {
    std::cout << std::format("Received message is [{}]", json) << std::endl;
  }
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
