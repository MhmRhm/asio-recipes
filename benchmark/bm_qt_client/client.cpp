#include "bm_qt_client/client.h"

#include <format>
#include <iostream>

Client::Client(QObject *parent)
    : QObject{parent}, m_socket{this}, m_timeoutTimer{this} {
  connect(&m_socket, &QTcpSocket::connected, this, [&]() {
    std::cout << std::format("Connectd to host.") << std::endl;
    m_socket.setSocketOption(QAbstractSocket::LowDelayOption, 1);
  });
  connect(&m_socket, &QTcpSocket::disconnected, this, [&]() {
    std::cout << std::format("Disconnected from host.") << std::endl;
    m_timeoutTimer.stop();
  });
  connect(&m_socket, &QTcpSocket::errorOccurred, this, [&]() {
    std::cout << std::format("Error {} occured: {}.",
                             static_cast<int>(m_socket.error()),
                             m_socket.errorString().toStdString())
              << std::endl;
    m_socket.disconnectFromHost();
  });
  connect(&m_socket, &QTcpSocket::readyRead, this, &Client::receiveResponse);

  m_timeoutTimer.setInterval(5000);
  m_timeoutTimer.setSingleShot(true);
  m_timeoutTimer.callOnTimeout(this, &Client::onTimeout);
}

bool Client::connectToHost(const QString &host, uint16_t port) {
  m_socket.connectToHost(host, port);
  return m_socket.waitForConnected(5000);
}

void Client::onTimeout() {
  std::cout << "Timeout occurred." << std::endl;
  m_socket.abort();
}

void Client::sendRequest() {
  m_timeoutTimer.start();

  m_dataLen = m_dataBuf.size();
  if (m_socket.write(reinterpret_cast<const char *>(&m_dataLen),
                     sizeof(m_dataLen)) == -1)
    return;
  if (m_socket.write(m_dataBuf) == -1)
    return;
  m_socket.flush();
  m_dataLen = 0;
  return;
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

  m_timeoutTimer.stop();
  emit responseReceived();
}
