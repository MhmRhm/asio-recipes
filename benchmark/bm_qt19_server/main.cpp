#include <format>
#include <iostream>
#include <source_location>
#include <thread>

#include <QByteArray>
#include <QCoreApplication>
#include <QSocketNotifier>
#include <QThread>

#include <google/protobuf/util/json_util.h>

#include "bm_qt19_server/server.h"
#include "bm_qt19_server_pb/WorkLoad.pb.h"

using namespace std;
using namespace google::protobuf::util;

void printMessage(qintptr socketDescriptor,
                  const myapp::WorkMessage &requestMsg) {
  std::string json{};
  auto status = MessageToJsonString(requestMsg, &json,
                                    JsonPrintOptions{.add_whitespace = true});
  if (status.ok()) {
    std::cout << std::format("Received request message from client {}:\n{}\n",
                             socketDescriptor, json);
  } else {
    std::cerr << "Failed to convert request message to JSON." << std::endl;
  }
}

void onRequest([[maybe_unused]] qintptr socketDescriptor, QByteArray &dataBuf) {
  // std::cout << std::format("Handling request from client {} in thread {}.",
  //                          socketDescriptor, QThread::currentThreadId())
  //           << std::endl;

  myapp::WorkMessage requestMsg{};
  requestMsg.ParseFromArray(dataBuf.constData(),
                            static_cast<int>(dataBuf.size()));
  // printMessage(socketDescriptor, requestMsg);

  // std::this_thread::sleep_for(
  //     std::chrono::milliseconds(requestMsg.work_request().workload()));

  myapp::WorkMessage responseMsg{};
  responseMsg.mutable_work_response()->set_job_id(
      requestMsg.work_request().job_id());
  responseMsg.mutable_work_response()->set_is_complete(true);

  dataBuf.resize(responseMsg.ByteSizeLong());
  responseMsg.SerializeToArray(dataBuf.data(),
                               static_cast<int>(dataBuf.size()));
}

int main(int argc, char *argv[]) {
  std::cout << std::format("Main thread: {}.", QThread::currentThreadId())
            << std::endl;

  QCoreApplication app{argc, argv};

  Server server{8172, &app, onRequest};
  server.start();

  std::jthread shutdownThread{[&app]() {
    std::cout << "Press ENTER to stop the server..." << std::endl;
    std::cin.get();
    app.quit();
  }};

  auto ret{app.exec()};
  std::cout << std::format("Total requests handled: {}",
                           server.getTotalRequests())
            << std::endl;
  return ret;
}
