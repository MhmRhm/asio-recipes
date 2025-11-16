#include "qt20_server/server.h"

#include <format>
#include <iostream>
#include <source_location>

#include <QThreadPool>

#include "qt20_server/client.h"

void Server::incomingConnection(qintptr socketDescriptor) {
  auto *client = new Client{socketDescriptor, m_onRequest, m_isStopping};
  QThreadPool::globalInstance()->start(client);
}
