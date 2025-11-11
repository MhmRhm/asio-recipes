#pragma once

#include <QObject>
#include <QTcpSocket>

class Client : public QObject {
  Q_OBJECT

private:
  QTcpSocket m_socket{};
  size_t m_dataLen{};
  QByteArray m_lenBuf{};
  QByteArray m_dataBuf{};
  std::function<void(QByteArray &)> m_onResponse{};

public:
  Client(QObject *parent = nullptr);
  virtual ~Client() = default;

  bool connectToHost(const QString &host, uint16_t port);
  bool isConnected() const {
    return m_socket.state() == QTcpSocket::ConnectedState;
  }
  QByteArray &dataBuffer() { return m_dataBuf; }
  void initiateCommunication(std::function<void(QByteArray &)> onResponse);
};
