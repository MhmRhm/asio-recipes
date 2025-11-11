#pragma once

#include <boost/asio.hpp>
#include <unordered_map>

#include <asio18_server/client.h>

class Server {
private:
  size_t m_dataLen{};
  boost::asio::mutable_buffer m_lenBuf{};
  boost::asio::streambuf m_dataBuf{};

  boost::asio::io_context &m_ioContext;
  boost::asio::ip::tcp::acceptor m_acceptor;

  int m_clientIdCounter{0};
  std::unordered_map<int, Client> m_clients{};
  std::function<void(boost::asio::streambuf &)> m_communicationHandler{};

public:
  Server(boost::asio::io_context &ioContext, unsigned short port,
         std::function<void(boost::asio::streambuf &)> communicationHandler);
  virtual ~Server() = default;

private:
  void checkOperation(const boost::system::error_code &ec,
                      size_t bytes_transferred, size_t expected_length);
  void onDisconnected(int clientId);
  void initiateAccept();
};
