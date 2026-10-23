#include <dobot_v4_bringup/http_server.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <sstream>

#include <dobot_v4_bringup/http_api_types.h>

namespace dobot_v4_bringup {
namespace {

std::string response(int status, const nlohmann::json& body)
{
    const std::string payload = body.dump();
    std::ostringstream out;
    out << "HTTP/1.1 " << status << " " << (status == 200 ? "OK" : "Error") << "\r\n"
        << "Content-Type: application/json\r\n"
        << "Content-Length: " << payload.size() << "\r\n"
        << "Connection: close\r\n\r\n" << payload;
    return out.str();
}

}  // namespace

HttpServer::HttpServer() = default;

HttpServer::~HttpServer()
{
    stop();
}

bool HttpServer::start(std::uint16_t port)
{
    if (running_) return false;
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) return false;

    int reuse = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    if (bind(server_fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0 || listen(server_fd_, 16) < 0) {
        close(server_fd_);
        server_fd_ = -1;
        return false;
    }
    port_ = port;
    running_ = true;
    worker_ = std::thread(&HttpServer::run, this);
    return true;
}

void HttpServer::stop()
{
    if (!running_.exchange(false)) return;
    shutdown(server_fd_, SHUT_RDWR);
    close(server_fd_);
    server_fd_ = -1;
    if (worker_.joinable()) worker_.join();
}

void HttpServer::registerGet(const std::string& path, Handler handler)
{
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    get_handlers_[path] = std::move(handler);
}

void HttpServer::registerPost(const std::string& path, Handler handler)
{
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    post_handlers_[path] = std::move(handler);
}

void HttpServer::run()
{
    while (running_) {
        const int client = accept(server_fd_, nullptr, nullptr);
        if (client >= 0) handleClient(client);
    }
}

void HttpServer::handleClient(int client_fd)
{
    char buffer[65536]{};
    const ssize_t size = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (size <= 0) {
        close(client_fd);
        return;
    }
    std::string request(buffer, static_cast<std::size_t>(size));
    const auto first_line_end = request.find("\r\n");
    const auto header_end = request.find("\r\n\r\n");
    if (first_line_end == std::string::npos || header_end == std::string::npos) {
        const std::string output = response(400, makeError(400, "malformed HTTP request"));
        send(client_fd, output.data(), output.size(), 0);
        close(client_fd);
        return;
    }
    std::istringstream first_line(request.substr(0, first_line_end));
    std::string method, path, version;
    first_line >> method >> path >> version;
    nlohmann::json query = nlohmann::json::object();
    const auto query_pos = path.find('?');
    if (query_pos != std::string::npos) {
        const std::string query_text = path.substr(query_pos + 1);
        path = path.substr(0, query_pos);
        std::istringstream query_stream(query_text);
        std::string item;
        while (std::getline(query_stream, item, '&')) {
            const auto equal_pos = item.find('=');
            const std::string key = item.substr(0, equal_pos);
            const std::string value = equal_pos == std::string::npos ? "" : item.substr(equal_pos + 1);
            if (!key.empty()) query[key] = value;
        }
    }
    const std::string body_text = request.substr(header_end + 4);
    nlohmann::json body = query;
    if (!body_text.empty()) {
        try { body = nlohmann::json::parse(body_text); }
        catch (...) { body = nlohmann::json(); }
    }

    Handler handler;
    {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        const auto& table = method == "GET" ? get_handlers_ : post_handlers_;
        const auto it = table.find(path);
        if (it != table.end()) handler = it->second;
    }
    nlohmann::json result;
    int status = 200;
    if (!handler) {
        status = 404;
        result = makeError(404, "route not found");
    } else {
        try { result = handler(body); }
        catch (const std::exception& ex) { status = 500; result = makeError(500, ex.what()); }
    }
    const std::string output = response(status, result);
    send(client_fd, output.data(), output.size(), 0);
    close(client_fd);
}

}  // namespace dobot_v4_bringup
