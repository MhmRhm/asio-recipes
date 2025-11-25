#include "bm_asio_server/server.h"
#include "bm_asio_server/client.h"

using namespace boost::asio;

Server::Server(boost::asio::io_context &ioContext, uint16_t port,
               std::function<void(int, boost::asio::streambuf &)> onRequest)
    : m_ioContext{ioContext},
      m_acceptor{m_ioContext, boost::asio::ip::tcp::endpoint(
                                  boost::asio::ip::tcp::v4(), port)},
      m_onRequest{onRequest} {
  initiateAccept();
}

void Server::onDisconnect(int clientId) {
  std::cout << std::format("Client with id {} disconnected.\n", clientId);
  m_clients.erase(clientId);
}

void Server::initiateAccept() {
  Client &client{
      (m_clients
           .try_emplace(m_clientIdCounter, m_ioContext, m_clientIdCounter,
                        m_totalRequests, [this](int id) { onDisconnect(id); })
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
        client.socket().set_option(boost::asio::ip::tcp::no_delay{true});
        client.initiateCommunication(m_onRequest);
      });
}
