#include <format>
#include <iostream>

#include <QCoreApplication>
#include <QSocketNotifier>

#include <google/protobuf/util/json_util.h>

#include "qt19_client/client.h"
#include "qt19_client_pb/WorkLoad.pb.h"

using namespace std;
using namespace google::protobuf::util;

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);
  Client client{};

  QObject::connect(&client, &Client::resposeReceived, [&]() {
    myapp::WorkMessage response_message{};
    response_message.ParseFromArray(
        client.dataBuffer().data(),
        static_cast<int>(client.dataBuffer().size()));

    std::string json{};
    auto status = MessageToJsonString(response_message, &json,
                                      JsonPrintOptions{.add_whitespace = true});
    if (status.ok()) {
      std::cout << "Received response message: " << json << std::endl;
    } else {
      std::cerr << "Failed to convert response message to JSON." << std::endl;
    }
  });

  uint32_t id{}, load{};
  QSocketNotifier stdinNotifier{fileno(stdin), QSocketNotifier::Read};
  QObject::connect(&stdinNotifier, &QSocketNotifier::activated, [&]() {
    if (!(std::cin >> load)) {
      app.quit();
      return;
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

    client.sendRequest();
  });

  return app.exec();
}
