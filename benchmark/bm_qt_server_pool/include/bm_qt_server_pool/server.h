#pragma once

#include <QTcpServer>
#include <QTimer>

class Server : public QTcpServer {
  Q_OBJECT
private:
  std::function<void(qintptr, QByteArray &)> m_onRequest;
  QTimer m_stopTimer{};
  std::atomic<int> m_totalRequests{};

public:
  explicit Server(QObject *parent,
                  std::function<void(qintptr, QByteArray &)> onRequest)
      : QTcpServer{parent}, m_onRequest{onRequest} {}
  ~Server() override = default;

  void disconnectClients() {
    QMetaObject::invokeMethod(&m_stopTimer, qOverload<>(&QTimer::start));
  }
  int getTotalRequests() const { return m_totalRequests.load(); }

protected:
  void incomingConnection(qintptr socketDescriptor) override;
};
