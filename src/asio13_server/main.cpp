#include <chrono>
#include <iostream>
#include <thread>

#include <asio13_server_pb/Person.pb.h>
#include <boost/asio.hpp>
#include <google/protobuf/util/json_util.h>

#include "asio13_server/session.h"

#define PORT 8174

using namespace boost::asio;
using namespace std::chrono_literals;

void check_op(boost::system::error_code ec, std::size_t rec_len,
              std::size_t exp_len) {
  if (ec.value())
    throw boost::system::system_error(ec);
  if (rec_len != exp_len)
    throw boost::system::system_error(
        boost::asio::error::operation_aborted,
        "Failed to read/write expected data length");
}

int run_server(Session &session, boost::asio::io_context &ioc) {
  Person me{};
  std::ostream data_out{&session.m_dataBuf};

  me.set_id(1000);
  me.set_name("Mohammad");
  me.set_email("rahimi.mhmmd@gmail.com");

  me.SerializeToOstream(&data_out);
  session.m_dataLen = session.m_dataBuf.size();

  auto protocol{ip::tcp::v4()};
  ip::tcp::acceptor acceptor{ioc};
  ip::tcp::endpoint endpoint{ip::address_v4::any(), PORT};

  try {
    acceptor.open(protocol);
    acceptor.bind(endpoint);
    acceptor.listen();
    acceptor.accept(session.m_socket);

    async_write(session.m_socket, session.m_lenBuf,
                [&](boost::system::error_code ec1, std::size_t len1) {
                  check_op(ec1, len1, sizeof(session.m_dataLen));
                  async_write(
                      session.m_socket, session.m_dataBuf,
                      [&](boost::system::error_code ec2, std::size_t len2) {
                        check_op(ec2, len2, session.m_dataLen);
                        // throw a fake exception to test error handling in bg
                        // thread
                        throw boost::system::system_error(
                            boost::asio::error::operation_aborted,
                            "Simulated exception in async write completion");
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
  auto res{run_server(session, ioc)};

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
