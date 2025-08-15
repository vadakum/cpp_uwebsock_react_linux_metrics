
#pragma once

#include "icollector.h"
#include "metric.h"
#include <chrono>
#include <queue>
#include <string>
#include <vector>

namespace cppserver {

struct CpuInfo {
  std::string _name = ""; // e.g., "cpu", "cpu0", "cpu1", etc.
  long _user = 0;
  long _nice = 0;
  long _system = 0;
  long _idle = 0;
  long _ioWait = 0;
  long _irq = 0;
  long _softIrq = 0;
  long _steal = 0;
  long _guest = 0;
  long _guestNice = 0;

  inline long getTotal() const {
    return _user + _nice + _system + _idle + _ioWait + _irq + _softIrq + _steal + _guest + _guestNice;
  }

  inline long getIdle() const { return _idle + _ioWait; }
};

struct CpuUsage {
  std::string _name = "";
  int _usagePercent = 0;
};

class CpuUsageCollector : public ICollector {
  using ProcStatEntries = std::vector<CpuInfo>;

public:
  void collect() override;
  nlohmann::json getJson() const override;
  std::string_view getMetricName() const override { return MetricName::CpuUsage; }

private:
  void readProcFile();

private:
  std::queue<ProcStatEntries> _procStatsQ;
  std::vector<CpuUsage> _cpuUsages;
  std::chrono::system_clock::time_point _lastReadTime{std::chrono::system_clock::now()};
};

} // namespace cppserver
