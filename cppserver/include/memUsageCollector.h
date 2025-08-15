
#pragma once

#include "icollector.h"
#include "metric.h"

#include <chrono>
#include <string>
#include <map>

namespace cppserver {

namespace MeminfoKeys {
inline constexpr std::string_view MemTotal = "MemTotal";
inline constexpr std::string_view MemFree = "MemFree";
inline constexpr std::string_view Buffers = "Buffers";
inline constexpr std::string_view Cached = "Cached";

// For now we are only interested in these 4 keys for calculating Total Used Memory:
// https://stackoverflow.com/questions/41224738/how-to-calculate-system-memory-usage-from-proc-meminfo-like-htop

} // namespace MeminfoKeys

struct MemUsage {
  int _usagePercent = 0;
};


class MemUsageCollector : public ICollector {

public:
  void collect() override;
  nlohmann::json getJson() const override;
  std::string_view getMetricName() const override { return MetricName::MemUsage; }

private:
  void readProcFile();

private:
  // Providing std::less<> for heterogeneous lookup. We are going to use .contains method
  // with string_view type argument
  std::map<std::string, long, std::less<>> _memInfo; 
  MemUsage _memUsage;
  std::chrono::system_clock::time_point _lastReadTime{std::chrono::system_clock::now()};
};

} // namespace cppserver