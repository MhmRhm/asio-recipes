#include <chrono>
#include <iostream>
#include <thread>

#include <asio13_client_pb/Person.pb.h>
#include <boost/asio.hpp>
#include <google/protobuf/util/json_util.h>

#include "asio13_client/session.h"

#define PORT 8174

using namespace boost::asio;
using namespace std::chrono_literals;

void check_op(boost::system::error_code ec, std::size_t rec_len,
              std::size_t exp_len) {
  if (ec.value())
    throw boost::system::system_error(ec, ec.message());
  if (rec_len != exp_len)
    throw boost::system::system_error(
        boost::asio::error::operation_aborted,
        "Failed to read/write expected data length");
}

void process_received_data(Session &session) {
  using namespace google::protobuf::util;

  std::istream data_in{&session.m_dataBuf};
  std::string json{};
  Person me{};

  me.ParseFromIstream(&data_in);

  auto status{
      MessageToJsonString(me, &json, JsonPrintOptions{.add_whitespace = true})};
  if (status.ok()) {
    std::cout << std::format("Received message is [{}]", json) << std::endl;
  }
}

int run_client(Session &session) {
  ip::tcp::endpoint endpoint{ip::make_address("127.0.0.1"), PORT};

  try {
    session.m_socket.open(endpoint.protocol());
    session.m_socket.connect(endpoint);

    async_read(session.m_socket, session.m_lenBuf,
               [&](boost::system::error_code ec1, std::size_t len1) {
                 check_op(ec1, len1, sizeof(session.m_dataLen));
                 async_read(
                     session.m_socket, session.m_dataBuf,
                     transfer_exactly(session.m_dataLen),
                     [&](boost::system::error_code ec2, std::size_t len2) {
                       check_op(ec2, len2, session.m_dataLen);
                       process_received_data(session);
                       session.m_dataBuf.consume(len2);
                     });
               });
  } catch (const boost::system::system_error &e) {
    std::cerr << "Error occured! Error code = " << e.code()
              << ". Message: " << e.what();
    return e.code().value();
  }

  return 0;
}

int main() {
  io_context ioc{};
  Session session{ioc};
  auto work_guard{make_work_guard(ioc)};

  std::thread bg_thread{[&]() {
    try {
      ioc.run();
    } catch (const boost::system::system_error &e) {
      std::cout << "Background thread caught exception. Error code = "
                << e.code() << ". Message: " << e.what() << std::endl;
    }
  }};
  auto res{run_client(session)};

  steady_timer timer{ioc, 10s};
  timer.async_wait([&](boost::system::error_code ec) {
    if (!ec) {
      std::cout << "Timer expired, resetting work guard..." << std::endl;
      work_guard.reset();
    }
  });

  std::cout << "Waiting for background thread to finish..." << std::endl;
  bg_thread.join();
  return res;
}
