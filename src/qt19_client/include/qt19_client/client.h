#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTimer>

class Client : public QObject {
  Q_OBJECT
private:
  QTcpSocket m_socket{};
  QTimer m_timeoutTimer{};
  QByteArray m_dataBuf{};
  uint64_t m_dataLen{};

signals:
  void responseReceived();

public:
  Client(QObject *parent = nullptr);
  virtual ~Client() = default;
  bool connectToHost(const QString &host, uint16_t port);
  bool isConnected() const {
    return m_socket.state() == QTcpSocket::ConnectedState;
  }
  QByteArray &dataBuffer() { return m_dataBuf; }

public slots:
  void sendRequest();

private slots:
  void receiveResponse();
  void onTimeout();
};
