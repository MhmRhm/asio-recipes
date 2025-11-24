## Introduction

Designing TCP clients and servers involves many factors, including the underlying platform, required performance, and expected workload. Some servers must support large numbers of concurrent connections, while others handle only a few but demand extremely low latency. These differing requirements often lead to very different design strategies.

The connection between a client and a server is also fragile. Failures can occur at any stage—name resolution, connection setup, or data transfer—and the implementation must be able to recover gracefully. Timeouts need to be handled reliably, and all of this should integrate smoothly into the application without blocking the main event loop.

In this work, I explore these challenges using two well-known networking frameworks: Boost.Asio and the Qt Networking module. Five projects were evaluated: two implement a TCP client and server using Boost.Asio, two provide equivalent implementations using Qt, and a fifth introduces an alternative design for the Qt-based server. All clients and servers are assumed to be non-blocking so they can run alongside the application's main event loop. After examining the design of these different approaches, we will test them to see how they perform in practice.

## Boost.Asio Client Implementation

Boost.Asio centers around the **io_context** class, regardless of whether your implementation is blocking or non-blocking. Every socket must be constructed with an `io_context` instance. After that, you can open a socket and use it in a blocking manner—for example, by calling `read_some` or `write_some`, both of which block until data arrives or is sent.

Non-blocking behavior is provided through the `async_*` variants of these methods. If your application requires a non-blocking design, you must keep the `io_context` running in a separate thread. In this model, whenever new data arrives, the `run` method triggers the corresponding callback provided to `async_read_some`.

```cpp
std::thread bg_thread{[&]() {
  while (!is_disconnected) {
    try {
      io_context.run();
    } catch (const boost::system::system_error &e) {
      printError(e.code());
    }
  }
}};
```

In my implementation, I used the free functions `async_read` and `async_write`. These functions rely heavily on a callback-based interface, which naturally pushes the callback approach into any code that interacts with them. The following provides an overview of the states a client transitions through when sending a request:

```cpp
void Client::initiateCommunication() {
  initiateSendRequest();
}
void Client::initiateSendRequest() {
  boost::asio::async_write(onLenSent);
}
void Client::onLenSent() {
  boost::asio::async_write(onDataSent);
}
void Client::onDataSent() {
  initiateReceiveResponse();
}
void Client::initiateReceiveResponse() {
  boost::asio::async_read(onLenReceived);
}
void Client::onLenReceived() {
  boost::asio::async_read(onDataReceived);
}
void Client::onDataReceived() {
  m_communicateHandler(m_dataBuf);
}
```

In contrast, the Qt framework is built around its Signal–Slot mechanism and enforces that event-driven model on the developer.

## Boost.Asio Server Implementation

Designing a server is more involved than writing a client, because a server must handle multiple connections concurrently. This is where `io_context` becomes especially valuable. By creating multiple threads and running `io_context::run` in each of them, you effectively create a thread pool. Each thread can then serve incoming client requests in parallel.

```cpp
for (size_t i{}; i < std::thread::hardware_concurrency(); i += 1) {
  threadPool.emplace_back([&]() {
    while (isRunning) {
      try {
        ioContext.run();
      } catch (const boost::system::system_error &e) {
        printError(e.code());
      }
    }
  });
}
```

Each client that connects to the server should have its own session object stored somewhere in memory. A session typically contains the accepted socket and the buffers used for communication. In my implementation, the server assigns each client a unique ID and stores its session in a map.

```cpp
std::unordered_map<int, Client> m_clients{};
```

When a client disconnects from the server, its corresponding entry is removed from the session map. The `Client` objects on the server side mirror the behavior of the `Client` objects on the client side. For example, if communication on the client begins with an `async_write`, the server starts the corresponding operation with an `async_read`.

A server implemented with Boost.Asio begins by listening for new connections on a passive socket (`acceptor`):

```cpp
// In Server class data members
boost::asio::ip::tcp::acceptor m_acceptor;

// In Server class constructor
m_acceptor{m_ioContext, boost::asio::ip::tcp::endpoint(
                                  boost::asio::ip::tcp::v4(), port)},
// In Server class method
void Server::initiateAccept() {
  Client &client{};
  m_acceptor.async_accept(
      client.socket(), [&](const boost::system::error_code &ec) {
        initiateAccept();
        client.initiateCommunication();
      });
}
```

## Qt Client Implementation

The same design principles apply to a client implemented with the Qt framework. The main difference is that Qt sockets operate in asynchronous mode by default. Whenever new data becomes available, the socket emits the `readyRead` signal. This makes transitioning between different phases of communication slightly more elegant in Qt compared to Boost.Asio, where you typically chain multiple callback functions.

```cpp
connect(&m_socket, &QTcpSocket::disconnected, this, [&]() {
  m_timeoutTimer.stop();
});
connect(&m_socket, &QTcpSocket::errorOccurred, this, [&]() {
  m_socket.disconnectFromHost();
});
connect(&m_socket, &QTcpSocket::readyRead, this, &Client::receiveResponse);
connect(&client, &Client::responseReceived, [&]() {
  // update app status
});
```

However, both approaches require roughly the same amount of code.

## Qt Server Implementation

When implementing a TCP server with Qt, there are two main options. The first is to move each incoming client session into its own thread. The second—closer to the Boost.Asio design—is to use a thread pool and dispatch communication tasks between the server and each client to that pool. These two approaches differ significantly. Let’s look at each one:

### Qt Server: One Thread per Client

When a new client connects to the server, the server receives a signal from Qt:

```cpp
connect(&server, &QTcpServer::newConnection, &server, [&]() {
  Client* client{new Client{listener.nextPendingConnection()}};
  connect(client, &Client::disconnected, client, &Client::deleteLater);
  client->start();
});
```

In this approach, the `Client` class inherits from `QThread`. When working with any multithreaded Qt application, it’s important to understand that each signal–slot connection has a sender and a receiver. The thread in which the slot executes is determined by the thread affinity of the **receiver** object:

```cpp
connect(sender, &Sender::signal, receiver, []() {
  // will execute in receiver thread
});
```

In Qt, a `QObject` cannot be moved to another `QThread` while it has a parent; Qt will ignore the move and issue a warning. To move an object safely, first remove its parent, then move it to the new thread, and plan carefully where its destructor will be called. In this implementation, the `Client` object is set to be deleted automatically when its socket disconnects from the server.

It’s also important to note that the `client` object itself lives in the server thread, while its internal TCP socket lives in the client thread. This means the `Client` constructor and destructor are executed in the server thread.

```cpp
Client::Client(QTcpSocket *socket, QObject *parent)
    : QThread{parent}, m_socket{socket} {
  m_socket->setParent(nullptr);
  m_socket->moveToThread(this);
}

void Client::run() {
  connect(m_socket, &QTcpSocket::disconnected, m_socket, [&]() {
    emit disconnected();
    quit();
  });
  connect(m_socket, &QTcpSocket::errorOccurred, m_socket, [&]() {
    m_socket->close();
  });
  connect(m_socket, &QTcpSocket::readyRead, m_socket,
          [&]() { receiveRequest();
  });
  connect(this, &Client::requestReceived, m_socket, [&]() {
    sendResponse();
  });

  exec();
  m_socket->deleteLater();
}
```

In general, due to Qt’s constraints on using signals and slots across threads, designing a server in Qt is more involved compared to Boost.Asio.

### Qt Server: Thread Pool Approach

In this approach, each server inherits from `QTcpServer` instead of aggregating it, and each client inherits from `QRunnable`. The `QRunnable` class represents a task that can later be assigned to a thread or a thread pool.

The advantage of this design is that it saves memory and CPU time by avoiding the overhead of creating a separate thread for each client. Additionally, load balancing and task destruction are handled automatically.

When the server detects a new connection, it creates a new task with the incoming socket descriptor and posts it to the thread pool.

```cpp
void Server::incomingConnection(qintptr socketDescriptor) {
  auto *client{
      new Client{socketDescriptor, m_onRequest, m_isStopping, m_totalRequests}};
  QThreadPool::globalInstance()->start(client);
}
```

The `QRunnable` object implements a `run` method, which executes in the assigned thread. Inside `run`, new signal–slot connections are established, and an event loop is started. The `run` method exits the event loop automatically when the client disconnects.

From a design perspective, this approach offers several advantages: we don’t need to manage the socket’s parent object or move it to a separate thread, and cleanup happens automatically when the event loop finishes.

The main drawback is that we need to periodically check whether the server has stopped. Because `QRunnable` does not inherit from `QObject`, server cannot emit signals to notify the clients. If this behavior is not desirable, an alternative is to emit a “server stopped” signal elsewhere and connect it to a data member in each client. This allows clients to detect the shutdown without relying on `QRunnable` directly.

```cpp
void Client::run() {
  QEventLoop loop{};
  QTcpSocket socket{};
  QTimer timer{};

  socket.setSocketDescriptor(m_socketDescriptor)

  QObject::connect(&socket, &QTcpSocket::disconnected, &socket,
                   [&]() { loop.quit(); });
  QObject::connect(&socket, &QTcpSocket::errorOccurred, &socket, [&]() {
    socket.close();
  });
  QObject::connect(&socket, &QTcpSocket::readyRead, &socket, [&]() {
    onReadyRead(socket);
  });

  QObject::connect(&timer, &QTimer::timeout, &timer, [&]() {
    if (m_isStopping.load()) {
      socket.close();
      loop.quit();
    }
  });

  loop.exec();
  std::cout << std::format("[{}] Client {} disconnected.",
                           QThread::currentThreadId(), m_socketDescriptor)
            << std::endl;
}
```

## Benchmarking Methodology

To avoid network latency affecting our benchmarking results, I ran both the client and server on the same host using the `localhost` network. The client initiates communication by sending a request to the server and starts the next round only after receiving the server’s response. Performance is measured as the number of requests handled by the server in 10 seconds. The requests and responses are simple Protobuf structures, serialized and deserialized on both the client and server sides to simulate a typical workflow.

**Benchmark results (requests per 10 seconds, averaged over 10 runs) on a Windows Intel machine:**

| Client      | Server                         | Requests/10s |
| ----------- | ------------------------------ | ---------- |
| Qt client   | Qt server (threads per client) | 43,096.4   |
| Qt client   | Qt server (thread pool)        | 48,084.3   |
| Qt client   | Asio server                    | 76,542.4   |
| Asio client | Qt server (threads per client) | 92,324.4   |
| Asio client | Qt server (thread pool)        | 104,661.6  |
| Asio client | Asio server                    | 91,281.5   |

**Benchmark results (requests per 10 seconds, averaged over 10 runs) on a Mac Mini M4 machine:**

| Client      | Server                         | Requests/10s |
| ----------- | ------------------------------ | ---------- |
| Qt client   | Qt server (threads per client) | 611,292.1  |
| Qt client   | Qt server (thread pool)        | 609,290.5  |
| Qt client   | Asio server                    | 264,311.3  |
| Asio client | Qt server (threads per client) | 230,921.8  |
| Asio client | Qt server (thread pool)        | 461,792.6  |
| Asio client | Asio server                    | 206,935.3  |

In experiments on virtual machines, the Boost.Asio implementation performed significantly worse. Each combination was run 10 times, and the values shown are the averages.
