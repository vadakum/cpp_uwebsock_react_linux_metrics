
#pragma once

#include <string>
#include <string_view>
#include <tp/json.hpp>

namespace cppserver {
    class ICollector {
    public:
        virtual void collect() = 0;
        virtual std::string_view getMetricName() const = 0;
        virtual nlohmann::json getJson() const = 0;
    };
} // namespace cppserver