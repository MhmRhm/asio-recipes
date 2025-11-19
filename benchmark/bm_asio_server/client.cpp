#include "bm_asio_server/client.h"

using namespace boost::asio;

bool Client::checkOperation(const boost::system::error_code &ec,
                            size_t bytes_transferred, size_t expected_length) {
  if (ec || bytes_transferred != expected_length) {
    m_serverTotalRequests += m_requestCount;
    m_onDisconnect(m_id);
    return false;
  }
  return true;
}

void Client::initiateCommunication(
    std::function<void(int, boost::asio::streambuf &)> onRequest) {
  m_onRequest = onRequest;
  initiateReceiveRequest();
}

void Client::initiateReceiveRequest() {
  m_dataLen = 0;
  m_lenBuf = boost::asio::mutable_buffer(&m_dataLen, sizeof(m_dataLen));

  boost::asio::async_read(
      m_socket, m_lenBuf,
      [this](const boost::system::error_code &ec, size_t bytes_transferred) {
        if (checkOperation(ec, bytes_transferred, sizeof(m_dataLen))) {
          onLenReceived();
        }
      });
}

void Client::onLenReceived() {
  boost::asio::async_read(
      m_socket, m_dataBuf, boost::asio::transfer_exactly(m_dataLen),
      [this](const boost::system::error_code &ec, size_t bytes_transferred) {
        if (checkOperation(ec, bytes_transferred, m_dataLen)) {
          onDataReceived();
        }
      });
}

void Client::onDataReceived() {
  m_onRequest(m_id, m_dataBuf);
  initiateSendResponse();
}

void Client::initiateSendResponse() {
  m_dataLen = m_dataBuf.size();
  m_lenBuf = boost::asio::mutable_buffer(&m_dataLen, sizeof(m_dataLen));

  boost::asio::async_write(
      m_socket, m_lenBuf,
      [this](const boost::system::error_code &ec, size_t bytes_transferred) {
        if (checkOperation(ec, bytes_transferred, sizeof(m_dataLen))) {
          onLenSent();
        }
      });
}

void Client::onLenSent() {
  boost::asio::async_write(
      m_socket, m_dataBuf,
      [this](const boost::system::error_code &ec, size_t bytes_transferred) {
        if (checkOperation(ec, bytes_transferred, m_dataLen)) {
          onDataSent();
        }
      });
}

void Client::onDataSent() {
  m_requestCount += 1;
  initiateReceiveRequest();
}
