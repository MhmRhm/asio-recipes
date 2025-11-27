#pragma once

#include <QByteArray>
#include <QEventLoop>
#include <QObject>
#include <QRunnable>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>

class Client : public QRunnable {
private:
  QByteArray m_dataBuf{};
  uint64_t m_dataLen{};
  qintptr m_socketDescriptor;
  std::function<void(qintptr, QByteArray &)> m_onRequest;
  QTimer &m_stopTimer;
  std::atomic<int> &m_totalRequests;

public:
  explicit Client(qintptr socketDescriptor,
                  std::function<void(qintptr, QByteArray &)> onRequest,
                  QTimer &stopTimer, std::atomic<int> &totalRequests)
      : m_socketDescriptor{socketDescriptor}, m_onRequest{onRequest},
        m_stopTimer{stopTimer}, m_totalRequests{totalRequests} {}

  void run() override;

private:
  void onReadyRead(QTcpSocket &socket);
};
