#include <dobot_v4_bringup/http_api_types.h>

namespace dobot_v4_bringup {

nlohmann::json makeSuccess(int code, const std::string& message, const nlohmann::json& data)
{
    return {{"success", true}, {"code", code}, {"message", message}, {"data", data}};
}

nlohmann::json makeError(int code, const std::string& message)
{
    return {{"success", false}, {"code", code}, {"message", message}, {"data", nullptr}};
}

}  // namespace dobot_v4_bringup
