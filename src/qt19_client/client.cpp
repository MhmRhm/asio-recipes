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
}

bool Client::connectToHost(const QString &host, uint16_t port) {
  m_socket.connectToHost(host, port);
  return m_socket.waitForConnected(5000);
}

void Client::initiateCommunication(
    std::function<void(QByteArray &)> onResponse) {
  m_onResponse = onResponse;

  m_dataLen = m_dataBuf.size();
  m_lenBuf =
      QByteArray{reinterpret_cast<const char *>(&m_dataLen), sizeof(m_dataLen)};
  qint64 len_bytes_written = m_socket.write(
      m_lenBuf.constData(), static_cast<qint64>(m_lenBuf.size()));

  if (len_bytes_written == -1) {
    std::cerr << "Failed to write length to socket." << std::endl;
    return;
  }
  if (!m_socket.waitForBytesWritten(5000)) {
    std::cerr << "Timeout while waiting for bytes to be written." << std::endl;
    return;
  }

  qint64 bytes_written = m_socket.write(m_dataBuf.constData(),
                                        static_cast<qint64>(m_dataBuf.size()));
  if (bytes_written == -1) {
    std::cerr << "Failed to write data to socket." << std::endl;
    return;
  }
  if (!m_socket.waitForBytesWritten(5000)) {
    std::cerr << "Timeout while waiting for bytes to be written." << std::endl;
    return;
  }

  // Wait for response
  if (!m_socket.waitForReadyRead(5000)) {
    std::cerr << "Timeout while waiting for response." << std::endl;
    return;
  }
  m_lenBuf = m_socket.read(sizeof(m_dataLen));
  m_dataLen = *reinterpret_cast<const size_t *>(m_lenBuf.constData());

  while (m_socket.bytesAvailable() != static_cast<int>(m_dataLen)) {
    if (!m_socket.waitForReadyRead(5000)) {
      std::cerr << "Timeout while waiting for response." << std::endl;
      return;
    }
  }
  m_dataBuf = m_socket.read(m_dataLen);

  if (onResponse) {
    onResponse(m_dataBuf);
  }
}