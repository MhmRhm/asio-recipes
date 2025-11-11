#include <format>
#include <iostream>

#include "qt19_client/client.h"
#include "qt19_client_pb/WorkLoad.pb.h"

#include <google/protobuf/util/json_util.h>

using namespace std;
using namespace google::protobuf::util;

int main() {
  bool is_disconnected{};
  Client client{};

  uint32_t load{};
  uint32_t id{};
  while (!is_disconnected && std::cin >> load) {
    if (load == static_cast<uint32_t>(-1)) {
      is_disconnected = true;
      continue;
    }

    if (!client.isConnected()) {
      client.connectToHost("127.0.0.1", 8172);
    }

    myapp::WorkMessage message{};
    myapp::WorkRequest *work_request = message.mutable_work_request();
    work_request->set_job_id(id += 1);
    work_request->set_workload(load);
    client.dataBuffer().resize(message.ByteSizeLong());
    message.SerializeToArray(client.dataBuffer().data(),
                             static_cast<int>(client.dataBuffer().size()));

    client.initiateCommunication([&](QByteArray &buffer) {
      myapp::WorkMessage response_message{};
      response_message.ParseFromArray(buffer.data(),
                                      static_cast<int>(buffer.size()));

      std::string json{};
      auto status = MessageToJsonString(
          response_message, &json, JsonPrintOptions{.add_whitespace = true});
      if (status.ok()) {
        std::cout << "Received response message: " << json << std::endl;
      } else {
        std::cerr << "Failed to convert response message to JSON." << std::endl;
      }
    });
  }

  return 0;
}
