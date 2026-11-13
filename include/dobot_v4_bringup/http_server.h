#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>

#include <nlohmann/json.hpp>

namespace dobot_v4_bringup {

class HttpServer {
public:
    using Handler = std::function<nlohmann::json(const nlohmann::json&)>;

    HttpServer();
    ~HttpServer();

    bool start(std::uint16_t port);
    void stop();
    void registerGet(const std::string& path, Handler handler);
    void registerPost(const std::string& path, Handler handler);

private:
    void run();
    void handleClient(int client_fd);

    std::uint16_t port_{0};
    int server_fd_{-1};
    std::atomic<bool> running_{false};
    std::thread worker_;
    std::mutex handlers_mutex_;
    std::map<std::string, Handler> get_handlers_;
    std::map<std::string, Handler> post_handlers_;
};

}  // namespace dobot_v4_bringup
