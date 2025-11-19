#include "bm_qt19_server/server.h"

#include <format>
#include <iostream>
#include <source_location>

#include <QTcpServer>
#include <QTcpSocket>

#include "bm_qt19_server/client.h"

Server::Server(uint16_t port, QObject *parent,
               std::function<void(qintptr, QByteArray &)> onRequest)
    : QThread{parent}, m_port{port}, m_onRequest{onRequest} {}

Server::~Server() {
  quit();
  wait();
}

void Server::run() {
  std::cout << std::format("Server thread: {}.", QThread::currentThreadId())
            << std::endl;

  QTcpServer listener{};
  connect(&listener, &QTcpServer::newConnection, &listener, [&]() {
    Client *client{new Client{listener.nextPendingConnection(), &listener,
                              m_onRequest, m_totalRequests}};
    connect(client, &Client::disconnected, client, &Client::deleteLater);
    client->start();
  });
  listener.listen(QHostAddress::Any, m_port);

  exec();
}
