#pragma once

#include <boost/asio.hpp>
#include <unordered_map>

#include <bm_asio18_server/client.h>

class Server {
private:
  boost::asio::io_context &m_ioContext;
  boost::asio::ip::tcp::acceptor m_acceptor;

  int m_clientIdCounter{};
  std::unordered_map<int, Client> m_clients{};
  std::function<void(int, boost::asio::streambuf &)> m_onRequest{};
  std::atomic<int> m_totalRequests{};

public:
  Server(boost::asio::io_context &ioContext, uint16_t port,
         std::function<void(int, boost::asio::streambuf &)> onRequest);
  virtual ~Server() = default;
  int getTotalRequests() const { return m_totalRequests.load(); }

private:
  void checkOperation(const boost::system::error_code &ec,
                      size_t bytes_transferred, size_t expected_length);
  void onDisconnect(int clientId);
  void initiateAccept();
};
