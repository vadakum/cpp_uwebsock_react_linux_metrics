
#pragma once

#include <App.h>
#include <iostream>
#include <string>
#include <thread>
#include <memory>
#include "icollector.h"


namespace cppserver {

const int PublishIntervalSec = 1;

struct SockData {
  std::string broadcastTopic {""};
};

class WebSocketServer {
  using CollectorVec = std::vector<std::unique_ptr<ICollector>>;
  
public:

  WebSocketServer(int port, bool verbose) : _listenPort(port), _verbose(verbose), _app(uWS::App()) {}
  void start();

private:
  static std::thread collectorDefPublisherThread(uWS::App& app, CollectorVec &collectors, bool verbose);

private:
  int _listenPort{};
  bool _verbose{};
  uWS::App _app; // uWS::App (or uWS::SSLApp)
};
} // namespace cppserver