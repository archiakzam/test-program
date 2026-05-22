/**
 * @file HttpServer.cpp
 * @brief Реализация простого HTTP-сервера.
 */

#include "HttpServer.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;
using json = nlohmann::json;

HttpServer::HttpServer(SystemStats& stats, int port)
    : m_stats(stats), m_port(port), m_running(false) {}

HttpServer::~HttpServer() { stop(); }

void HttpServer::start() {
    if (m_running) return;
    m_running = true;
    m_thread = thread(&HttpServer::run, this);
}

void HttpServer::stop() {
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
}

/**
 * @brief Отправляет строку через сокет.
 * @param fd Сокет.
 * @param str Данные.
 */
void HttpServer::sendString(int fd, const string& str) {
    send(fd, str.c_str(), str.size(), 0);
}

/**
 * @brief Читает содержимое файла в строку.
 * @param filename Путь к файлу.
 * @return Содержимое файла или пустая строка при ошибке.
 */
string HttpServer::readFile(const string& filename) {
    ifstream file(filename, ios::binary | ios::ate);
    if (!file.is_open()) return "";
    streamsize size = file.tellg();
    file.seekg(0, ios::beg);
    string buffer(size, '\0');
    if (file.read(&buffer[0], size)) return buffer;
    return "";
}

/**
 * @brief Отдаёт статический файл (index.html, style.css, script.js).
 * @param clientAcc Сокет клиента.
 * @param path Запрошенный путь.
 */
void HttpServer::serveStaticFile(int clientAcc, const string& path) {
    string content;
    string mime;
    if (path == "/" || path == "/index.html") {
        content = readFile("www/index.html");
        mime = "text/html";
    } else if (path == "/style.css") {
        content = readFile("www/style.css");
        mime = "text/css";
    } else if (path == "/script.js") {
        content = readFile("www/script.js");
        mime = "application/javascript";
    } else {
        sendString(clientAcc, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
        return;
    }
    if (content.empty()) {
        sendString(clientAcc, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
        return;
    }
    ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: " << mime << "\r\n"
             << "Content-Length: " << content.size() << "\r\n"
             << "Connection: close\r\n\r\n"
             << content;
    sendString(clientAcc, response.str());
}

/**
 * @brief Формирует JSON со статистикой системы.
 * @return Строка JSON.
 */
string HttpServer::buildStatsJSON() {
    json j;
    auto now = chrono::system_clock::now();
    j["timestamp"] = chrono::duration_cast<chrono::seconds>(now.time_since_epoch()).count();
    j["cpu"] = m_stats.getCPU();
    j["ram"] = m_stats.getRAM();
    j["cpu_temp"] = m_stats.getCPUTemp();
    
    json disks_arr = json::array();
    for (const auto& d : m_stats.getDisks()) {
        disks_arr.push_back({
            {"mount", d.mountPoint},
            {"usage", d.usagePercent}
        });
    }
    j["disks"] = disks_arr;
    return j.dump();
}

/**
 * @brief Формирует JSON со списком процессов.
 * @return Строка JSON.
 */
string HttpServer::buildProcessesJSON() {
    auto procs = m_stats.getProcesses();
    json j = json::array();
    for (const auto& p : procs) {
        j.push_back({
            {"pid", p.pid},
            {"name", p.name},
            {"cpu", p.cpu_percent},
            {"mem_mb", p.memory_mb}
        });
    }
    return j.dump();
}

/**
 * @brief Обрабатывает один HTTP-запрос.
 * @param clientAcc Сокет клиента.
 * @param request Строка запроса.
 */
void HttpServer::handleRequest(int clientAcc, const string& request) {
    istringstream req_stream(request);
    string method, path, version;
    req_stream >> method >> path >> version;
    
    if (method != "GET") {
        sendString(clientAcc, "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n");
        return;
    }
    
    if (path == "/stats") {
        string json_data = buildStatsJSON();
        ostringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: application/json\r\n"
                 << "Content-Length: " << json_data.size() << "\r\n"
                 << "Connection: close\r\n\r\n"
                 << json_data;
        sendString(clientAcc, response.str());
        return;
    }
    
    if (path == "/processes") {
        string json_data = buildProcessesJSON();
        ostringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: application/json\r\n"
                 << "Content-Length: " << json_data.size() << "\r\n"
                 << "Connection: close\r\n\r\n"
                 << json_data;
        sendString(clientAcc, response.str());
        return;
    }
    
    serveStaticFile(clientAcc, path);
}

/**
 * @brief Основной цикл сервера: bind, listen, accept.
 * 
 * Работает в отдельном потоке. Принимает соединения, читает запрос,
 * вызывает handleRequest, закрывает сокет.
 */
void HttpServer::run() {
    int servSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (servSocket < 0) {
        cerr << "[HTTP] Failed to create socket\n";
        return;
    }
    int opt = 1;
    setsockopt(servSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(m_port);
    
    if (bind(servSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "[HTTP] Bind failed on port " << m_port << "\n";
        close(servSocket);
        return;
    }
    
    if (listen(servSocket, 5) < 0) {
        cerr << "[HTTP] Listen failed\n";
        close(servSocket);
        return;
    }
    
    cout << "[HTTP] Server started on http://localhost:" << m_port << "\n";
    
    while (m_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int clientAcc = accept(servSocket, (struct sockaddr*)&client_addr, &client_len);
        if (clientAcc < 0) {
            if (m_running) cerr << "[HTTP] Accept failed\n";
            continue;
        }
        
        char buffer[4096];
        int n = recv(clientAcc, buffer, sizeof(buffer) - 1, 0);
        if (n > 0) {
            buffer[n] = '\0';
            handleRequest(clientAcc, string(buffer));
        }
        close(clientAcc);
    }
    close(servSocket);
    cout << "[HTTP] Server stopped\n";
}