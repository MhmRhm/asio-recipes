#pragma once

#include <QByteArray>
#include <QTcpSocket>
#include <QThread>

class Client : public QThread {
  Q_OBJECT
private:
  QByteArray m_dataBuf{};
  uint64_t m_dataLen{};
  QTcpSocket *m_socket{};
  std::function<void(qintptr, QByteArray &)> m_onRequest;

signals:
  void requestReceived();
  void disconnected();

public:
  explicit Client(QTcpSocket *socket, QObject *parent,
                  std::function<void(qintptr, QByteArray &)> onRequest);
  virtual ~Client();

protected:
  void run() override;

private:
  void receiveRequest();
  void sendResponse();
};
