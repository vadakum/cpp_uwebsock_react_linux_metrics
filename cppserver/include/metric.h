
#pragma once

#include <string_view>

namespace cppserver {
// Each metric name will be a different broadcast channel
namespace MetricName {
inline constexpr std::string_view CpuUsage = "cpu_usage";
inline constexpr std::string_view MemUsage = "mem_usage";
} // namespace MetricName
} // namespace cppserver
