#pragma once

#include <functional>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

class Client {
private:
  uint64_t m_dataLen{};
  boost::asio::mutable_buffer m_lenBuf{};
  boost::asio::streambuf m_dataBuf{};
  boost::asio::ip::tcp::socket m_socket;
  std::function<void(boost::asio::streambuf &)> m_communicateHandler{};
  std::function<void()> m_disconnectHandler{};
  boost::asio::steady_timer m_timeoutTimer;

public:
  Client(boost::asio::io_context &io_context,
         std::function<void()> disconnectHandler)
      : m_socket{io_context}, m_disconnectHandler{disconnectHandler},
        m_timeoutTimer{io_context} {}
  virtual ~Client() = default;

  boost::system::error_code connect(const std::string &address,
                                    unsigned short port);
  bool isConnected() { return m_socket.is_open(); }
  boost::asio::streambuf &dataBuffer() { return m_dataBuf; }
  void initiateCommunication(
      std::function<void(boost::asio::streambuf &)> communicationHandler);

private:
  bool checkOperation(const boost::system::error_code &ec,
                      size_t bytes_transferred, size_t expected_length);
  void checkTimeout(const boost::system::error_code &ec);
  void initiateSendRequest();
  void onLenSent();
  void onDataSent();
  void initiateReceiveResponse();
  void onLenReceived();
  void onDataReceived();
};
