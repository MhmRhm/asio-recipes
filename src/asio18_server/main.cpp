#include <chrono>
#include <thread>

#include <boost/asio.hpp>
#include <google/protobuf/util/json_util.h>

#include "asio18_server/server.h"
#include "asio18_server_pb/WorkLoad.pb.h"

using namespace boost::asio;
using namespace google::protobuf::util;

void printError(const boost::system::error_code &ec) {
  std::cout << std::format("Error code = {}.\nMessage: {}\n", ec.to_string(),
                           ec.what());
}

void onRequest(int clientID, boost::asio::streambuf &dataBuf) {
  std::istream is(&dataBuf);
  myapp::WorkMessage requestMsg{};
  requestMsg.ParseFromIstream(&is);

  std::string json{};
  auto status = MessageToJsonString(requestMsg, &json,
                                    JsonPrintOptions{.add_whitespace = true});
  if (status.ok()) {
    std::cout << std::format("Received request message from client {}:\n{}\n",
                             clientID, json);
  } else {
    std::cerr << "Failed to convert request message to JSON." << std::endl;
  }

  std::this_thread::sleep_for(
      std::chrono::milliseconds(requestMsg.work_request().workload()));

  std::ostream os(&dataBuf);
  myapp::WorkMessage responseMsg{};
  responseMsg.mutable_work_response()->set_job_id(
      requestMsg.work_request().job_id());
  responseMsg.mutable_work_response()->set_is_complete(true);
  responseMsg.SerializeToOstream(&os);
}

int main() {
  std::atomic_bool isRunning{true};

  io_context ioContext{};
  auto workGuard{make_work_guard(ioContext)};
  std::vector<std::thread> threadPool{};
  // for (size_t i{}; i < 1; i += 1) {
  for (size_t i{}; i < std::thread::hardware_concurrency(); i += 1) {
    threadPool.emplace_back([&]() {
      while (isRunning) {
        try {
          ioContext.run();
        } catch (const boost::system::system_error &e) {
          printError(e.code());
        }
      }
    });
  }
  Server server{ioContext, 8172, onRequest};

  std::string line{};
  while (std::cin >> line)
    ;

  isRunning = false;
  workGuard.reset();
  ioContext.stop();
  for (auto &t : threadPool) {
    t.join();
  }
}
