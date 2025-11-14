#pragma once

#include <QObject>
#include <QTcpSocket>

class Client : public QObject {
  Q_OBJECT

private:
  QTcpSocket m_socket{};
  QByteArray m_dataBuf{};
  uint64_t m_dataLen{};

signals:
  void resposeReceived();

public:
  Client(QObject *parent = nullptr);
  virtual ~Client() = default;
  bool connectToHost(const QString &host, uint16_t port);
  bool isConnected() const {
    return m_socket.state() == QTcpSocket::ConnectedState;
  }
  QByteArray &dataBuffer() { return m_dataBuf; }

public slots:
  bool sendRequest();

private slots:
  void receiveResponse();
};
