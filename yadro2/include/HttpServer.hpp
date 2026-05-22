/**
 * @file HttpServer.hpp
 * @brief Простой HTTP-сервер для отдачи JSON и статических файлов.
 */

#pragma once
#include "SystemStats.hpp"
#include <thread>
#include <atomic>

/**
 * @class HttpServer
 * @brief HTTP-сервер на сокетах (один поток, блокирующий accept).
 * 
 * Поддерживает:
 * - GET /stats -> JSON с метриками (CPU, RAM, температура, диски)
 * - GET /processes -> JSON со списком процессов
 * - GET /, /index.html, /style.css, /script.js -> статические файлы из папки www/
 * 
 * Работает в отдельном потоке, управляется флагом m_running.
 */
class HttpServer {
public:
    /**
     * @brief Конструктор.
     * @param stats Ссылка на хранилище метрик (живёт дольше сервера).
     * @param port Номер порта для прослушивания.
     */
    HttpServer(SystemStats& stats, int port);
    ~HttpServer();

    /// Запускает поток сервера.
    void start();

    /// Останавливает сервер и дожидается завершения потока.
    void stop();

private:
    SystemStats& m_stats;          ///< ссылка на общие данные
    int m_port;                    ///< порт
    std::thread m_thread;          ///< поток, в котором работает run()
    std::atomic<bool> m_running;   ///< флаг для остановки

    void run();                    ///< основной цикл сервера: bind, listen, accept
    void handleRequest(int client_fd, const std::string& request); ///< парсинг запроса
    void serveStaticFile(int client_fd, const std::string& path);   ///< отдача статики
    void sendString(int fd, const std::string& str);                ///< удобная обёртка send()
    std::string readFile(const std::string& filename);              ///< чтение всего файла
    std::string buildStatsJSON();       ///< формирует JSON для /stats
    std::string buildProcessesJSON();   ///< формирует JSON для /processes
};