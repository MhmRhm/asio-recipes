#include "qt19_client/client.h"

#include <format>
#include <iostream>

Client::Client(QObject *parent) : QObject{parent}, m_socket{this} {
  connect(&m_socket, &QTcpSocket::connected, this,
          []() { std::cout << std::format("Connectd to host.") << std::endl; });
  connect(&m_socket, &QTcpSocket::disconnected, this, []() {
    std::cout << std::format("Disconnected from host.") << std::endl;
  });
  connect(&m_socket, &QTcpSocket::errorOccurred, this, [this]() {
    std::cout << std::format("Error {} occured: {}.",
                             static_cast<int>(m_socket.error()),
                             m_socket.errorString().toStdString())
              << std::endl;
    m_socket.disconnectFromHost();
  });
  connect(&m_socket, &QTcpSocket::readyRead, this, &Client::receiveResponse);
}

bool Client::connectToHost(const QString &host, uint16_t port) {
  m_socket.connectToHost(host, port);
  return m_socket.waitForConnected(5000);
}

bool Client::sendRequest() {
  m_dataLen = m_dataBuf.size();
  if (m_socket.write(reinterpret_cast<const char *>(&m_dataLen),
                     sizeof(m_dataLen)) == -1)
    return false;
  if (m_socket.write(m_dataBuf) == -1)
    return false;
  m_socket.flush();
  m_dataLen = 0;
  return true;
}

void Client::receiveResponse() {
  if (!m_dataLen &&
      m_socket.bytesAvailable() >= static_cast<int64_t>(sizeof(m_dataLen)))
    m_socket.read(reinterpret_cast<char *>(&m_dataLen), sizeof(m_dataLen));
  if (!m_dataLen || m_socket.bytesAvailable() < static_cast<int64_t>(m_dataLen))
    return;

  m_dataBuf.resize(m_dataLen);
  m_socket.read(m_dataBuf.data(), m_dataLen);
  m_dataLen = 0;

  emit resposeReceived();
}
