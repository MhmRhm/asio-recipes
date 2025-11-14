#include "qt20_server/client.h"

#include <format>
#include <iostream>
#include <source_location>

Client::Client(QTcpSocket *socket, QObject *parent,
               std::function<void(qintptr, QByteArray &)> onRequest)
    : QThread{parent}, m_socket{socket}, m_onRequest{onRequest} {
  m_socket->setParent(nullptr);
  m_socket->moveToThread(this);
}

Client::~Client() {
  quit();
  wait();
}

void Client::run() {
  std::cout << std::format("Client thread: {}.", QThread::currentThreadId())
            << std::endl;

  connect(m_socket, &QTcpSocket::disconnected, m_socket, [&]() {
    std::cout << std::format("Disconnected from host.") << std::endl;
    emit disconnected();
    quit();
  });
  connect(m_socket, &QTcpSocket::errorOccurred, m_socket, [&]() {
    std::cout << std::format("Error {} occured: {}.",
                             static_cast<int>(m_socket->error()),
                             m_socket->errorString().toStdString())
              << std::endl;
    m_socket->disconnectFromHost();
  });
  connect(m_socket, &QTcpSocket::readyRead, m_socket,
          [&]() { receiveRequest(); });
  connect(this, &Client::requestReceived, m_socket, [&]() { sendResponse(); });

  exec();
  m_socket->deleteLater();
}

void Client::receiveRequest() {
  if (!m_dataLen &&
      m_socket->bytesAvailable() >= static_cast<int64_t>(sizeof(m_dataLen)))
    m_socket->read(reinterpret_cast<char *>(&m_dataLen), sizeof(m_dataLen));
  if (!m_dataLen ||
      m_socket->bytesAvailable() < static_cast<int64_t>(m_dataLen))
    return;

  m_dataBuf.resize(m_dataLen);
  m_socket->read(m_dataBuf.data(), m_dataLen);
  m_dataLen = 0;

  emit requestReceived();
}

void Client::sendResponse() {
  m_onRequest(m_socket->socketDescriptor(), m_dataBuf);

  m_dataLen = m_dataBuf.size();
  if (m_socket->write(reinterpret_cast<const char *>(&m_dataLen),
                      sizeof(m_dataLen)) == -1)
    return;
  if (m_socket->write(m_dataBuf) == -1)
    return;
  m_socket->flush();
  m_dataLen = 0;
  return;
}
