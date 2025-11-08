#include <chrono>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

using namespace boost::asio;
using namespace std::chrono_literals;

int main() {
  io_context ioc{};

  steady_timer t1{ioc};
  steady_timer t2{ioc};
  steady_timer t3{ioc};

  t1.expires_after(1s);
  t2.expires_after(2s);
  t3.expires_after(2s);

  // std::this_thread::sleep_for(5s);

  t1.async_wait([&](boost::system::error_code ec) {
    std::cout << std::format("Timer 1 stoped with ec = {}, {}", ec.value(),
                             ec.what())
              << std::endl;
    t3.cancel_one();
  });
  t2.async_wait([](boost::system::error_code ec) {
    std::cout << std::format("Timer 2 stoped with ec = {}, {}", ec.value(),
                             ec.what())
              << std::endl;
  });
  t3.async_wait([](boost::system::error_code ec) {
    std::cout << std::format("Timer 3.1 stoped with ec = {}, {}", ec.value(),
                             ec.what())
              << std::endl;
  });
  t3.async_wait([](boost::system::error_code ec) {
    std::cout << std::format("Timer 3.2 stoped with ec = {}, {}", ec.value(),
                             ec.what())
              << std::endl;
  });

  ioc.run();
}
