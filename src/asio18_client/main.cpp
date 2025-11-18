#include <chrono>
#include <thread>

#include <boost/asio.hpp>
#include <google/protobuf/util/json_util.h>

#include "asio18_client/client.h"
#include "asio18_client_pb/WorkLoad.pb.h"

using namespace boost::asio;
using namespace google::protobuf::util;

void printError(const boost::system::error_code &ec) {
  std::cout << std::format("Error code = {}.\nMessage: {}\n", ec.to_string(),
                           ec.what());
}

void printResponse(myapp::WorkMessage &response_message) {
  std::string json{};
  auto status = MessageToJsonString(response_message, &json,
                                    JsonPrintOptions{.add_whitespace = true});
  if (status.ok()) {
    std::cout << "Received response message: " << json << std::endl;
  } else {
    std::cerr << "Failed to convert response message to JSON." << std::endl;
  }
}

void sendRequest(Client &client, uint32_t id, uint32_t load,
                 std::atomic_bool &is_disconnected) {
  std::ostream os{&client.dataBuffer()};
  myapp::WorkMessage message{};
  myapp::WorkRequest *work_request = message.mutable_work_request();
  work_request->set_job_id(id);
  work_request->set_workload(load);
  message.SerializeToOstream(&os);

  client.initiateCommunication([&, new_id = id + 1](streambuf &buffer) {
    std::istream is{&buffer};
    myapp::WorkMessage response_message{};
    response_message.ParseFromIstream(&is);
    printResponse(response_message);

    // if (!is_disconnected)
    //   sendRequest(client, new_id, 0, is_disconnected);
  });
}

int main() {
  std::atomic_bool is_disconnected{};
  uint32_t load{}, id{};
  io_context io_context;
  auto work_guard{make_work_guard(io_context)};
  Client client{io_context, [&]() {
                  std::cout << "Disconnected from server." << std::endl;
                }};

  boost::system::error_code ec = client.connect("127.0.0.1", 8172);
  if (ec) {
    printError(ec);
    return ec.value();
  }

  std::thread bg_thread{[&]() {
    while (!is_disconnected) {
      try {
        io_context.run();
      } catch (const boost::system::system_error &e) {
        printError(e.code());
      }
    }
  }};

  // sendRequest(client, id + 1, 0, is_disconnected);

  while (true) {
    if (!(std::cin >> load)) {
      is_disconnected = true;
      break;
    }

    if (!client.isConnected()) {
      boost::system::error_code ec1 = client.connect("127.0.0.1", 8172);
      if (ec1) {
        printError(ec1);
        return ec1.value();
      }
    }

    sendRequest(client, id += 1, load, is_disconnected);
  }

  work_guard.reset();
  bg_thread.join();
  return 0;
}
