#pragma once

#include <QThread>

class Server : public QThread {
  Q_OBJECT
private:
  uint16_t m_port{};
  std::function<void(qintptr, QByteArray &)> m_onRequest;
  std::atomic<int> m_totalRequests{};

public:
  explicit Server(uint16_t port, QObject *parent,
                  std::function<void(qintptr, QByteArray &)> onRequest);
  virtual ~Server();
  int getTotalRequests() const { return m_totalRequests.load(); }

protected:
  void run() override;
};
