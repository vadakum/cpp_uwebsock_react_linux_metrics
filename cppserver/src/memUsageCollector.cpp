

#include "memUsageCollector.h"
#include "tp/json.hpp"

#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

const std::string procFilePath = "/proc/meminfo";

void cppserver::MemUsageCollector::readProcFile() {

  // File Format (Key: Value kB)
  // MemTotal:       15929256 kB
  // MemFree:         8841912 kB
  // MemAvailable:   11777460 kB
  // Buffers:            5012 kB
  // Cached:          3542988 kB
  // .....

  std::ifstream procFileStream(procFilePath);
  if (!procFileStream.is_open()) {
    throw std::runtime_error(std::format("Failed to open {} for reading", procFilePath));
  }

  std::string line;
  while (std::getline(procFileStream, line)) {
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) {
      continue; // Skip malformed lines
    }

    std::string key = line.substr(0, colonPos);
    size_t valueStart = colonPos + 1;
    size_t valueEnd = line.find(" kB", valueStart); // Assuming " kB" unit
    std::string valueStr;
    if (valueEnd != std::string::npos) {
      valueStr = line.substr(valueStart, valueEnd - valueStart);
    } else {
      valueStr = line.substr(valueStart); // " kB" unit not found, take till end
    }
    try {
      _memInfo[key] = std::stol(valueStr);
    } catch (const std::invalid_argument &e) {
      std::cout << "Warning: Could not parse value for key '" << key << "': " << e.what() << std::endl;
    } catch (const std::out_of_range &e) {
      std::cout << "Warning: Value out of range for key '" << key << "': " << e.what() << std::endl;
    }
  }
}

void cppserver::MemUsageCollector::collect() {
  auto now = std::chrono::system_clock::now();
  // Avoid collecting too frequently
  if (now - _lastReadTime < std::chrono::milliseconds(1000)) {
    return;
  }

  try {
    readProcFile();
  } catch (const std::runtime_error &e) {
    std::cerr << "Error collecting memory usage: " << e.what() << std::endl;
    return; 
  }

  if (_memInfo.contains(MeminfoKeys::MemTotal) && _memInfo.contains(MeminfoKeys::MemFree) &&
      _memInfo.contains(MeminfoKeys::Buffers) && _memInfo.contains(MeminfoKeys::Cached)) {
    // Note: Directly passing a std::string_view as a key to std::map<std::string, int>::operator[]
    // is not supported by default. Contains and find should work as std::less<> compfn was provided
    const long memTotal = _memInfo.find(MeminfoKeys::MemTotal)->second; // _memInfo[MeminfoKeys::MemTotal];
    const long memFree = _memInfo.find(MeminfoKeys::MemFree)->second;   // _memInfo[MeminfoKeys::MemFree];
    const long buffers = _memInfo.find(MeminfoKeys::Buffers)->second;   // _memInfo[MeminfoKeys::Buffers];
    const long cached = _memInfo.find(MeminfoKeys::Cached)->second;     // _memInfo[MeminfoKeys::Cached];
    const long usedMem = memTotal - memFree - buffers - cached;
    const int usage_percent = static_cast<int>((usedMem * 100) / memTotal);
    _memUsage._usagePercent = usage_percent;
  }

  _lastReadTime = std::chrono::system_clock::now();
}

nlohmann::json cppserver::MemUsageCollector::getJson() const {
  nlohmann::json j;
  j["metric_name"] = static_cast<std::string>(getMetricName()); // casting: just to be safe
  j["ts"] = std::chrono::duration_cast<std::chrono::seconds>(_lastReadTime.time_since_epoch()).count();
  j["data"] = nlohmann::json::object({{"usage", _memUsage._usagePercent}});
  return j;
}
