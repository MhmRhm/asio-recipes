#include "bm_qt_server_pool/server.h"

#include <format>
#include <iostream>
#include <source_location>

#include <QThreadPool>

#include "bm_qt_server_pool/client.h"

void Server::incomingConnection(qintptr socketDescriptor) {
  auto *client{
      new Client{socketDescriptor, m_onRequest, m_isStopping, m_totalRequests}};
  QThreadPool::globalInstance()->start(client);
}
