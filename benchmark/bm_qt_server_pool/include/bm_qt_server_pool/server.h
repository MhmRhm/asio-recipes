#pragma once

#include <QTcpServer>

class Server : public QTcpServer {
  Q_OBJECT
private:
  std::function<void(qintptr, QByteArray &)> m_onRequest;
  std::atomic<bool> m_isStopping{};
  std::atomic<int> m_totalRequests{};

public:
  explicit Server(QObject *parent,
                  std::function<void(qintptr, QByteArray &)> onRequest)
      : QTcpServer{parent}, m_onRequest{onRequest} {}
  ~Server() override = default;

  void disconnectClients() { m_isStopping.store(true); }
  int getTotalRequests() const { return m_totalRequests.load(); }

protected:
  void incomingConnection(qintptr socketDescriptor) override;
};
