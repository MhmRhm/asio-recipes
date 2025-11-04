#include "asio14_server/server.h"
#include "asio14_server/client.h"

using namespace boost::asio;

Server::Server(
    boost::asio::io_context &ioContext, unsigned short port,
    std::function<void(boost::asio::streambuf &)> communicationHandler)
    : m_ioContext{ioContext},
      m_acceptor{m_ioContext, boost::asio::ip::tcp::endpoint(
                                  boost::asio::ip::tcp::v4(), port)},
      m_communicationHandler{communicationHandler} {
  initiateAccept();
}

void Server::onDisconnected(int clientId) {
  std::cout << std::format("Client with id {} disconnected.\n", clientId);
  m_clients.erase(clientId);
}

void Server::initiateAccept() {
  Client &client{
      (m_clients
           .try_emplace(m_clientIdCounter, m_ioContext, m_clientIdCounter,
                        [this](int id) { onDisconnected(id); })
           .first->second)};
  m_clientIdCounter += 1;
  m_acceptor.async_accept(
      client.socket(), [&](const boost::system::error_code &ec) {
        std::cout << std::format("New client connected with id {}.\n",
                                 client.id());
        if (ec) {
          throw boost::system::system_error(ec);
        }
        initiateAccept();
        client.initiateCommunication(m_communicationHandler);
      });
}
