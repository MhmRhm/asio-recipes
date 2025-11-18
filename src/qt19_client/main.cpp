#include <format>
#include <iostream>

#include <QCoreApplication>
#include <QSocketNotifier>

#include <google/protobuf/util/json_util.h>

#include "qt19_client/client.h"
#include "qt19_client_pb/WorkLoad.pb.h"

using namespace std;
using namespace google::protobuf::util;

void sendRequest(Client &client, uint32_t id, uint32_t load) {
  myapp::WorkMessage message{};
  myapp::WorkRequest *workRequest = message.mutable_work_request();
  workRequest->set_job_id(id);
  workRequest->set_workload(load);
  client.dataBuffer().resize(message.ByteSizeLong());
  message.SerializeToArray(client.dataBuffer().data(),
                           static_cast<int>(client.dataBuffer().size()));

  client.sendRequest();
}

void printResponse(myapp::WorkMessage &responseMsg) {
  std::string json{};
  auto status = MessageToJsonString(responseMsg, &json,
                                    JsonPrintOptions{.add_whitespace = true});
  if (status.ok()) {
    std::cout << "Received response message: " << json << std::endl;
  } else {
    std::cerr << "Failed to convert response message to JSON." << std::endl;
  }
}

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);
  uint32_t id{}, load{};
  Client client{};

  QObject::connect(&client, &Client::responseReceived, [&]() {
    myapp::WorkMessage responseMsg{};
    responseMsg.ParseFromArray(client.dataBuffer().data(),
                               static_cast<int>(client.dataBuffer().size()));
    printResponse(responseMsg);
  });

  client.connectToHost("127.0.0.1", 8172);

  // QObject::connect(&client, &Client::responseReceived,
  //                  [&]() { sendRequest(client, id += 1, 0); });
  // sendRequest(client, id, 0);

  QSocketNotifier stdinNotifier{fileno(stdin), QSocketNotifier::Read};
  QObject::connect(&stdinNotifier, &QSocketNotifier::activated, [&]() {
    if (!(std::cin >> load)) {
      app.quit();
      return;
    }

    if (!client.isConnected()) {
      client.connectToHost("127.0.0.1", 8172);
    }

    sendRequest(client, id += 1, load);
  });

  return app.exec();
}
