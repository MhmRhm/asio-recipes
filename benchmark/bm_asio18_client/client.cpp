#include "bm_asio18_client/client.h"

#include <chrono>

boost::system::error_code Client::connect(const std::string &address,
                                          unsigned short port) {
  boost::asio::ip::tcp::endpoint endpoint{
      boost::asio::ip::make_address(address), port};

  boost::system::error_code ec;
  m_socket.open(endpoint.protocol(), ec);
  if (!ec) {
    m_socket.connect(endpoint, ec);
  }
  return ec;
}

bool Client::checkOperation(const boost::system::error_code &ec,
                            size_t bytes_transferred, size_t expected_length) {
  if (ec || bytes_transferred != expected_length) {

    m_disconnectHandler();
    return false;
  }
  return true;
}

void Client::checkTimeout(const boost::system::error_code &ec) {
  if (!ec) {
    std::cout << "Timeout occurred." << std::endl;
    m_socket.cancel();
    m_socket.close();
  }
}

void Client::initiateCommunication(
    std::function<void(boost::asio::streambuf &)> communicationHandler) {
  m_communicateHandler = communicationHandler;
  initiateSendRequest();
}

void Client::initiateSendRequest() {
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

void Client::onDataSent() { initiateReceiveResponse(); }

void Client::initiateReceiveResponse() {
  m_dataLen = 0;
  m_lenBuf = boost::asio::mutable_buffer(&m_dataLen, sizeof(m_dataLen));

  m_timeoutTimer.expires_after(std::chrono::seconds{5});
  m_timeoutTimer.async_wait(
      [this](const boost::system::error_code &ec) { checkTimeout(ec); });
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
        m_timeoutTimer.cancel();
        if (checkOperation(ec, bytes_transferred, m_dataLen)) {
          onDataReceived();
        }
      });
}

void Client::onDataReceived() { m_communicateHandler(m_dataBuf); }
