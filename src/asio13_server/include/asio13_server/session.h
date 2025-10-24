#pragma once

#include <boost/asio.hpp>

struct Session final {
  Session(boost::asio::io_context &ioc) : m_socket{ioc} {}

  size_t m_dataLen{};
  boost::asio::streambuf m_dataBuf{};
  boost::asio::mutable_buffer m_lenBuf{&m_dataLen, sizeof(m_dataLen)};
  boost::asio::ip::tcp::socket m_socket;
};
