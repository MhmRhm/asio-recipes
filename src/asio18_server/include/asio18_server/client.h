#pragma once

#include <functional>

#include <boost/asio.hpp>

class Client {
private:
  int m_id{};
  uint64_t m_dataLen{};
  boost::asio::mutable_buffer m_lenBuf{};
  boost::asio::streambuf m_dataBuf{1 << 20}; // 1 MB buffer
  boost::asio::ip::tcp::socket m_socket;
  std::function<void(int, boost::asio::streambuf &)> m_onRequest{};
  std::function<void(int)> m_onDisconnect{};

public:
  Client(boost::asio::io_context &io_context, int id,
         std::function<void(int)> onDisconnect)
      : m_id{id}, m_socket{io_context}, m_onDisconnect{onDisconnect} {}
  virtual ~Client() = default;

  int id() const { return m_id; }
  boost::asio::streambuf &dataBuffer() { return m_dataBuf; }
  boost::asio::ip::tcp::socket &socket() { return m_socket; }
  void initiateCommunication(
      std::function<void(int, boost::asio::streambuf &)> onRequest);

private:
  bool checkOperation(const boost::system::error_code &ec,
                      size_t bytes_transferred, size_t expected_length);
  void initiateReceiveRequest();
  void onLenReceived();
  void onDataReceived();
  void initiateSendResponse();
  void onLenSent();
  void onDataSent();
};
