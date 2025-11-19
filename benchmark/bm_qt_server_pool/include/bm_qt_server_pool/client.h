#pragma once

#include <QByteArray>
#include <QEventLoop>
#include <QObject>
#include <QRunnable>
#include <QTcpSocket>
#include <QThread>

class Client : public QRunnable {
private:
  QByteArray m_dataBuf{};
  uint64_t m_dataLen{};
  qintptr m_socketDescriptor;
  std::function<void(qintptr, QByteArray &)> m_onRequest;
  std::atomic<bool> &m_isStopping;
  std::atomic<int> &m_totalRequests;

public:
  explicit Client(qintptr socketDescriptor,
                  std::function<void(qintptr, QByteArray &)> onRequest,
                  std::atomic<bool> &isStopping,
                  std::atomic<int> &totalRequests)
      : m_socketDescriptor{socketDescriptor}, m_onRequest{onRequest},
        m_isStopping{isStopping}, m_totalRequests{totalRequests} {}

  void run() override;

private:
  void onReadyRead(QTcpSocket &socket);
};
