#include <chrono>
#include <thread>

#include <boost/asio.hpp>
#include <google/protobuf/util/json_util.h>

#include "asio14_server/server.h"
#include "asio14_server_pb/WorkLoad.pb.h"

using namespace boost::asio;
using namespace google::protobuf::util;

void printError(const boost::system::error_code &ec) {
  std::cout << std::format("Error code = {}.\nMessage: {}\n", ec.to_string(),
                           ec.what());
}

void communicationHandler(boost::asio::streambuf &dataBuf) {
  std::istream is(&dataBuf);
  myapp::WorkMessage requestMsg;
  requestMsg.ParseFromIstream(&is);

  std::this_thread::sleep_for(
      std::chrono::milliseconds(requestMsg.work_request().workload()));

  std::ostream os(&dataBuf);
  myapp::WorkMessage responseMsg;
  responseMsg.mutable_work_response()->set_job_id(
      requestMsg.work_request().job_id());
  responseMsg.mutable_work_response()->set_is_complete(true);
  responseMsg.SerializeToOstream(&os);
}

int main() {
  io_context io_context{};
  auto work_guard{make_work_guard(io_context)};
  std::vector<std::thread> threadPool{};
  // for (size_t i{}; i < std::thread::hardware_concurrency(); i += 1) {
  for (size_t i{}; i < 1; i += 1) {
    threadPool.emplace_back([&io_context]() {
      try {
        io_context.run();
      } catch (const boost::system::system_error &e) {
        printError(e.code());
      }
    });
  }
  Server server{io_context, 8172, communicationHandler};

  std::string line{};
  while (std::cin >> line)
    ;

  work_guard.reset();
  io_context.stop();
  for (auto &t : threadPool) {
    t.join();
  }
}
