#include "bm_qt20_server/client.h"

#include <format>
#include <iostream>

#include <QTimer>

void Client::run() {
  QEventLoop loop{};
  QTcpSocket socket{};
  QTimer timer{};
  int requestCount{};

  if (!socket.setSocketDescriptor(m_socketDescriptor)) {
    std::cout << std::format("[{}] Failed to set socket descriptor: {}",
                             QThread::currentThreadId(),
                             socket.errorString().toStdString())
              << std::endl;
    return;
  }

  std::cout << std::format("[{}] Client {} connected.",
                           QThread::currentThreadId(), m_socketDescriptor)
            << std::endl;

  QObject::connect(&socket, &QTcpSocket::disconnected, &socket,
                   [&]() { loop.quit(); });
  QObject::connect(&socket, &QTcpSocket::errorOccurred, &socket, [&]() {
    std::cout << std::format("[{}] Error {} occured: {}.",
                             QThread::currentThreadId(),
                             static_cast<int>(socket.error()),
                             socket.errorString().toStdString())
              << std::endl;
    socket.close();
  });
  QObject::connect(&socket, &QTcpSocket::readyRead, &socket, [&]() {
    onReadyRead(socket);
    requestCount += 1;
  });

  timer.setInterval(1000);
  timer.setSingleShot(false);
  QObject::connect(&timer, &QTimer::timeout, &timer, [&]() {
    if (m_isStopping.load()) {
      socket.close();
      loop.quit();
    }
  });
  timer.start();

  loop.exec();
  m_totalRequests += requestCount;
  std::cout << std::format("[{}] Client {} disconnected.",
                           QThread::currentThreadId(), m_socketDescriptor)
            << std::endl;
}

void Client::onReadyRead(QTcpSocket &socket) {
  if (!m_dataLen &&
      socket.bytesAvailable() >= static_cast<int64_t>(sizeof(m_dataLen)))
    socket.read(reinterpret_cast<char *>(&m_dataLen), sizeof(m_dataLen));
  if (!m_dataLen || socket.bytesAvailable() < static_cast<int64_t>(m_dataLen))
    return;

  m_dataBuf.resize(m_dataLen);
  socket.read(m_dataBuf.data(), m_dataLen);

  m_onRequest(socket.socketDescriptor(), m_dataBuf);

  m_dataLen = m_dataBuf.size();
  if (socket.write(reinterpret_cast<const char *>(&m_dataLen),
                   sizeof(m_dataLen)) == -1)
    return;
  if (socket.write(m_dataBuf) == -1)
    return;
  socket.flush();
  m_dataLen = 0;
  return;
}
