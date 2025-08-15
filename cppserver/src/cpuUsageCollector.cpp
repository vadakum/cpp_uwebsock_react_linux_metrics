

#include "cpuUsageCollector.h"
#include "tp/json.hpp"
#include <format>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <iostream>

const std::string procFilePath = "/proc/stat";
/*
 * Note: Not to be used in production! All this istringstream, vector/queue
 * allocation, and deallocation causes a performance overhead. In fact, we may be bumping up
 * the CPU and memory usage a bit while trying to collect some harmless metrics :-).
 * But here, we just want to keep things simple and readable.
 */

void cppserver::CpuUsageCollector::readProcFile() {

  std::ifstream procFileStream(procFilePath);
  if (!procFileStream.is_open()) {
    throw std::runtime_error(std::format("Failed to open {} for reading", procFilePath));
  }

  std::string line;
  ProcStatEntries pse;
  while (std::getline(procFileStream, line)) {
    if (line.find("cpu") == 0) { // or std::string_view sv(line); if (sv.starts_with("cpu"))
      std::istringstream iss(line);
      CpuInfo cpuInfo;
      iss >> cpuInfo._name >> cpuInfo._user >> cpuInfo._nice >> cpuInfo._system >> cpuInfo._idle >> cpuInfo._ioWait >>
          cpuInfo._irq >> cpuInfo._softIrq >> cpuInfo._steal >> cpuInfo._guest >> cpuInfo._guestNice;
      pse.push_back(cpuInfo);
    }
  }
  _procStatsQ.push(pse);
}

void cppserver::CpuUsageCollector::collect() {
  auto now = std::chrono::system_clock::now();
  // Avoid collecting too frequently
  if (now - _lastReadTime < std::chrono::milliseconds(1000)) {
    return;
  }

  try {
    readProcFile();
  } catch (const std::runtime_error &e) {
    std::cerr << "Error collecting CPU usage: " << e.what() << std::endl;
    return; 
  }

  // Keep only the last two entries in the queue to calculate usage
  while (_procStatsQ.size() > 2) {
    _procStatsQ.pop();
  }

  // Ensure we have at least two entries to calculate usage
  if (_procStatsQ.size() != 2) {
    return;
  }

  // To calculate CPU usage, we need two snapshots of the /proc/stat file.
  // The formula is:
  //
  // CPU Usage % = ( (total - idle) - (prev_total - prev_idle) ) / (total - prev_total) * 100
  //
  // Where:
  //   - `total` is the sum of all time fields for the current snapshot.
  //   - `idle` is the idle time for the current snapshot.
  //   - `prev_total` and `prev_idle` are the corresponding values from the previous snapshot.
  //
  // This simplifies to: (total_delta - idle_delta) / total_delta * 100
  _cpuUsages.clear();
  ProcStatEntries prevStat = _procStatsQ.front();
  ProcStatEntries currStat = _procStatsQ.back();

  for (size_t i = 0; i < currStat.size(); ++i) {
    if (i < prevStat.size()) {
      const CpuInfo &prevCpu = prevStat[i];
      const CpuInfo &currCpu = currStat[i];
      auto totalTimePrev = prevCpu.getTotal();
      auto totalTimeCurr = currCpu.getTotal();
      auto idleTimePrev = prevCpu.getIdle();
      auto idleTimeCurr = currCpu.getIdle();

      if (totalTimeCurr > totalTimePrev) {
        const double totalDelta = totalTimeCurr - totalTimePrev;
        const double idleDelta = idleTimeCurr - idleTimePrev;
        int usagePercent = static_cast<int>(((totalDelta - idleDelta) * 100.0) / totalDelta);
        _cpuUsages.push_back({currCpu._name, usagePercent});
      }
    }
  }
  _lastReadTime = std::chrono::system_clock::now();
}

nlohmann::json cppserver::CpuUsageCollector::getJson() const {
  nlohmann::json j;
  j["metric_name"] = static_cast<std::string>(getMetricName()); // casting: just to be safe
  j["ts"] = std::chrono::duration_cast<std::chrono::seconds>(_lastReadTime.time_since_epoch()).count();
  j["data"] = nlohmann::json::array();
  auto &ja = j["data"];
  for (const auto &cu : _cpuUsages) {
    ja.push_back({{"name", cu._name}, {"usage", cu._usagePercent}});
  }
  return j;
}
