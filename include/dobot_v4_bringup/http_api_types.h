#pragma once

#include <string>

#include <nlohmann/json.hpp>

namespace dobot_v4_bringup {

nlohmann::json makeSuccess(int code, const std::string& message, const nlohmann::json& data);
nlohmann::json makeError(int code, const std::string& message);

}  // namespace dobot_v4_bringup
