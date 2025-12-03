## Introduction

Designing TCP clients and servers involves many factors, including the underlying platform, required performance, and expected workload. Some servers must support large numbers of concurrent connections, while others handle only a few but demand extremely low latency. These differing requirements often lead to very different design strategies.

The connection between a client and a server is also fragile. Failures can occur at any stage—name resolution, connection setup, or data transfer—and the implementation must be able to recover gracefully. Timeouts need to be handled reliably, and all of this should integrate smoothly into the application without blocking the main event loop.

In this work, I explore these challenges using two well-known networking frameworks: Boost.Asio and the Qt Networking module. Five projects were evaluated: two implement a TCP client and server using Boost.Asio, two provide equivalent implementations using Qt, and a fifth introduces an alternative design for the Qt-based server. All clients and servers are assumed to be non-blocking so they can run alongside the application's main event loop. After examining the design of these different approaches, we will test them to see how they perform in practice.

You can find the source code on GitHub at [https://github.com/MhmRhm/asio-recipes/tree/main/benchmark](https://github.com/MhmRhm/asio-recipes/tree/main/benchmark).

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

One complication with this approach is notifying clients that the server is about to shut down. Since `QRunnable` does not inherit from `QObject`, the server cannot emit signals directly to the client runnables. A practical solution is to place a `QObject` member inside the `Server` class, pass a reference to this object to each client runnable, and connect one of its signals to the client’s event loop `quit()` slot.

In the example below, I use a `QTimer` as the shared `QObject`. When the server is about to shut down, it starts the timer with an interval of 0. The timer’s `timeout` signal is then emitted, causing each connected client’s event loop to exit.

```cpp
void Client::run() {
  QEventLoop loop{};
  QTcpSocket socket{};

  socket.setSocketDescriptor(m_socketDescriptor)

  QObject::connect(&socket, &QTcpSocket::disconnected, &loop,
                   &QEventLoop::quit);
  QObject::connect(&socket, &QTcpSocket::errorOccurred, &socket, [&]() {
    socket.close();
  });
  QObject::connect(&socket, &QTcpSocket::readyRead, &socket, [&]() {
    onReadyRead(socket);
  });

  QObject::connect(&m_stopTimer, &QTimer::timeout, &loop, &QEventLoop::quit);

  loop.exec();
  std::cout << std::format("[{}] Client {} disconnected.",
                           QThread::currentThreadId(), m_socketDescriptor)
            << std::endl;
}
```

With this change, the server application can now perform a graceful shutdown:

```cpp
// server.h
void disconnectClients() {
  QMetaObject::invokeMethod(&m_stopTimer, qOverload<>(&QTimer::start));
}

// main.cpp
std::jthread shutdownThread{[&app, &server, pool]() {
  std::cout << "Press ENTER to stop the server..." << std::endl;
  std::cin.get();

  QMetaObject::invokeMethod(&server, &Server::close);
  while (server.isListening())
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

  server.disconnectClients();
  pool->waitForDone();
  app.quit();
}};
```

## Timeouts

Both libraries implement timeouts using timers.
In **Boost.Asio**, when a timer expires it triggers a callback, where you can close the socket and notify the user gracefully:

```cpp
void Client::checkTimeout(const boost::system::error_code &ec) {
  if (!ec) {
    m_socket.cancel();
    m_socket.close();
  }
}

void Client::initiateReceiveResponse() {
  m_timeoutTimer.expires_after(std::chrono::seconds{5});
  m_timeoutTimer.async_wait(
      [this](const boost::system::error_code &ec) { checkTimeout(ec); });
  boost::asio::async_read();
}
```

In **Qt**, after sending the request the client starts a single-shot timer.
If the timer expires, the `timeout` signal triggers the timeout handler, which can cleanly abort the socket connection. The socket’s `disconnected` signal is then used to stop the timer:

```cpp
void Client::onTimeout() {
  std::cout << "Timeout occurred." << std::endl;
  m_socket.abort();
}

connect(&m_socket, &QTcpSocket::disconnected, this, [&]() {
  m_timeoutTimer.stop();
});
m_timeoutTimer.setInterval(5000);
m_timeoutTimer.setSingleShot(true);
m_timeoutTimer.callOnTimeout(this, &Client::onTimeout);
```

## Benchmarking Methodology

To avoid network latency affecting our benchmarking results, for single-threaded tests I ran both the client and server on the same host using the `localhost` network. The client initiates communication by sending a request to the server and starts the next round only after receiving the server’s response. Performance is measured as the number of requests handled by the server in 10 seconds. The requests and responses are simple Protobuf structures, serialized and deserialized on both the client and server sides to simulate a typical workflow.

I also ran the tests with the `TCP_NODELAY` option enabled on the sockets. Below has the benchmarking results for an Asio server running on a Windows machine and clients running on a Mac Mini M4. By changing only this single option, we achieved more than a **90× improvement** in performance—from **2,097.5 requests** to **191,174.2 requests**.

**Benchmark results (requests in 10 seconds, averaged over 10 runs):**

| Server (Intel)                 | Client (Intel) | Default      | TCP_NODELAY |
| ------------------------------ | -------------- | ------------ | ----------- |
| Asio server                    | Asio client    | 91,281.5     | 74,550.3    |
| Asio server                    | Qt client      | 76,542.4     | 76,532.9    |
| Qt server (threads per client) | Qt client      | 43,096.4     | 47,855.6    |
| Qt server (threads per client) | Asio client    | 92,324.4     | 93,776.1    |
| Qt server (thread pool)        | Qt client      | 48,084.3     | 48,819.3    |
| Qt server (thread pool)        | Asio client    | 104,661.6    | 116,093.5   |

| Server (M4)                    | Client (M4) | Default      | TCP_NODELAY |
| ------------------------------ | ----------- | ------------ | ----------- |
| Asio server                    | Asio client | 206,935.3    | 200,891.7   |
| Asio server                    | Qt client   | 264,311.3    | 339,798.3   |
| Qt server (threads per client) | Asio client | 230,921.8    | 224,973.7   |
| Qt server (threads per client) | Qt client   | 611,292.1    | 601,358.5   |
| Qt server (thread pool)        | Asio client | 461,792.6    | 462,812.3   |
| Qt server (thread pool)        | Qt client   | 609,290.5    | 615,127.0   |

The following describes a multithreaded test scenario. I connected two machines directly using a LAN cable, assigned static IP addresses to both, and then ran 10 client instances on one machine with a single server instance running on the other.

**Benchmark results (requests from 10 clients in 10 seconds, averaged over 10 runs):**

| Server (Intel)                 | Clients (M4) | Default      | TCP_NODELAY |
| ------------------------------ | ------------ | ------------ | ----------- |
| Asio server                    | Asio client  | 2,097.5      | 191,174.2   |
| Asio server                    | Qt client    | 104,227.3    | 207,779.3   |
| Qt server (threads per client) | Asio client  | 2,109.5      | 135,172.2   |
| Qt server (threads per client) | Qt client    | 140,278.7    | 137,324.6   |
| Qt server (thread pool)        | Asio client  | 4,218.5      | 136,204.3   |
| Qt server (thread pool)        | Qt client    | 138,677.6    | 137,728.3   |

| Server (M4)                    | Clients (Intel) | Default   | TCP_NODELAY |
| ------------------------------ | --------------- | ----------| ----------- |
| Asio server                    | Asio client     | 2,029.1   | 200,925.7   |
| Asio server                    | Qt client       | 2,026.4   | 108,777.2   |
| Qt server (threads per client) | Asio client     | 109,134.0 | 214,652.5   |
| Qt server (threads per client) | Qt client       | 106,598.7 | 117,003.9   |
| Qt server (thread pool)        | Asio client     | 216,184.2 | 267,041.5   |
| Qt server (thread pool)        | Qt client       | 109,924.4 | 115,258.7   |

## Closing Thoughts

Designing both the client and server sides of an application comes with many challenges, and even a clean, well-structured design does not guarantee good performance. The main lesson is that performance is something you discover through experimentation, not something you can assume.

As the benchmark results showed, different operating systems behave differently, and even two very similar concurrent Qt TCP server designs can diverge in performance. We also saw how a single socket option can significantly change throughput and latency. Factors such as data volume, latency requirements, and the number of concurrent clients all influence what “optimal” really means.

Ultimately, you cannot know whether you’re getting the best performance from your hardware unless you benchmark different approaches in a controlled environment—free from outside interference—so that every candidate can be evaluated fairly and accurately.
