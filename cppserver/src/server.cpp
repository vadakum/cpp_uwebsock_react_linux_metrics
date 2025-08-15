
#include "server.h"
#include "cpuUsageCollector.h"
#include "memUsageCollector.h"
#include "metric.h"
#include "tp/json.hpp"
#include <iostream>
#include <string>
#include <thread>

/*
=========
Protocol:
=========

Subscription Request example:
{
  "type": "sub"
  "metric_name": "cpu_usage" or "mem_usage" or ... // as defined in metric.h => MetricName
}

Unsubscription Request example:
{
  "type": "unsub"
  "metric_name": "cpu_usage" or "mem_usage" or ... // as defined in metric.h => MetricName
}

Subscription Response:
{
  "type": "sub_response",
  "status" : "success" or "failure",
  "message": "..."
}

Unsubscription Response:
{
  "type": "unsub_response",
  "status" : "failure",
  "message": "..."
}

Error Response:
{
  "type": "error_response",
  "status" : "success" or "failure",
  "message": "..."
}

Metrics streaming json structure:
{
  "type" : "sub_stream",
  "metric_name": "cpu_usage" or "mem_usage" or ... // as defined in metric.h => MetricName
  "ts" : <unix epoch>
  "data" : {}  // check getJson method of respective collector
}
*/

/*
 * This is our worker thread
 */
std::thread cppserver::WebSocketServer::collectorDefPublisherThread(uWS::App &app, CollectorVec &collectors,
                                                                    bool verbose) {

  return (std::thread([&app, &collectors, verbose]() {
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

    std::cout << "-> Publishing interval is " << PublishIntervalSec << " seconds\n";
    std::cout << "-> Starting collector thread...\n";

    while (true) {
      // Collect metrics
      for (auto &collector : collectors) {
        collector->collect();
      }

      // Throttle publishing
      std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
      if (std::chrono::duration_cast<std::chrono::seconds>(end - start).count() >= PublishIntervalSec) {

        std::map<std::string, std::string> metrics;
        for (auto &collector : collectors) {
          auto j = collector->getJson();
          j["type"] = "sub_stream";
          metrics.insert(std::make_pair(collector->getMetricName(), j.dump()));
        }

        // Schedule our lambda on the main event loop thread, so that uWS can
        // safely publish the message (uWS::App is single-threaded)
        app.getLoop()->defer([&app, metrics, verbose]() {
          for (const auto &[broadcastTopic /*metricName*/, jsonStr] : metrics) {
            if (verbose) {
              std::cout << "[Pub: (client count:" << app.numSubscribers(broadcastTopic) << ")] -> " << jsonStr
                        << std::endl;
            }
            auto result = app.publish(broadcastTopic, jsonStr, uWS::OpCode::TEXT);
            if (verbose) {
              std::cout << "[Pub status to clients: " << (result ? "✅" : "❌") << "]" << std::endl;
            }
          }
        });

        start = end;
      }
      // We go slow, collect metrics every second
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }));
}

/*
 * Wrapper functions for websocket Handlers (.open, .message and .close)
 * Note: auto in the parameter list: Abbreviated function template C++20
 */
inline void processOpen(auto *ws) { std::cout << "-> Client connected\n"; }

inline void processMessage(auto *ws, std::string_view message, uWS::OpCode opCode) {
  std::cout << "-> Received message from client: " << message << std::endl;
  nlohmann::json j;
  auto obj = j.parse(message);

  if (obj.contains("type") && obj.contains("metric_name")) {
    auto messageType = obj["type"].get<std::string_view>();
    if (messageType == "sub") {
      const auto broadcastTopic = obj["metric_name"].get<std::string>();
      ws->subscribe(broadcastTopic);
      ws->getUserData()->broadcastTopic = broadcastTopic;
      std::cout << "-> Subscribed to: " << broadcastTopic << std::endl;
      nlohmann::json js = {{"type", "sub_response"}, {"status", "success"}, {"message", "Successfully subscribed"}};
      ws->send(js.dump(), opCode, false);
      return;
    }
    if (messageType == "unsub") {
      const auto broadcastTopic = obj["metric_name"].get<std::string>();
      ws->unsubscribe(broadcastTopic);
      ws->getUserData()->broadcastTopic = "";
      std::cout << "-> Unsubscribed from: " << broadcastTopic << std::endl;
      nlohmann::json js = {{"type", "unsub_response"}, {"status", "success"}, {"message", "Successfully unsubscribed"}};
      ws->send(js.dump(), opCode, false);
      return;
    }
  }
  nlohmann::json je = {{"type", "error_response"},
                       {"status", "failure"},
                       {"message", "expected 'type' [e.g: sub] and 'metric_name' keys in the message"}};
  ws->send(je.dump(), opCode, false);
}

inline void processClose(auto *ws, int code, std::string_view message) {
  (void)message; // silence unused parameter warning
  auto &broadcastTopic = ws->getUserData()->broadcastTopic;
  if (!broadcastTopic.empty()) {
    ws->unsubscribe(broadcastTopic);
    std::cout << "-> Unsubscibed from: " << broadcastTopic << std::endl;
    broadcastTopic = "";
  }
  std::cout << "-> Client disconnected with code: " << code << std::endl;
}

/*
 * Server entry point
 */
void cppserver::WebSocketServer::start() {

  // For now just configure collectors here, this could be dynamic or config driven
  CollectorVec collectors;
  collectors.emplace_back(std::make_unique<cppserver::CpuUsageCollector>());
  collectors.emplace_back(std::make_unique<cppserver::MemUsageCollector>());

  std::thread collectorThread = WebSocketServer::collectorDefPublisherThread(_app, collectors, _verbose);

  _app.ws<SockData>("/*",
                    {
                        // Handlers
                        .open = [](auto *ws) { processOpen(ws); },
                        .message = [](auto *ws, std::string_view message,
                                      uWS::OpCode opCode) { processMessage(ws, message, opCode); },
                        .close = [](auto *ws, int code, std::string_view message) { processClose(ws, code, message); },
                    });

  _app.listen(_listenPort, [port = _listenPort](auto *lsock) {
    if (lsock) {
      std::cout << "-> Websocket Server is listening on port " << port << std::endl;
    }
  });

  _app.run();
  collectorThread.join();
}