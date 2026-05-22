/**
 * @file Application.hpp
 * @brief Главный класс приложения, управляющий потоками и жизненным циклом.
 */

#pragma once
#include "SystemStats.hpp"
#include "HttpServer.hpp"
#include <vector>
#include <thread>
#include <memory>

class ICollector;

/**
 * @class Application
 * @brief Оркестрирует сборщики метрик и HTTP-сервер.
 * 
 * Создаёт все collector-ы, запускает их в отдельных потоках, стартует HTTP-сервер,
 * ожидает нажатия Enter и корректно завершает все потоки.
 */
class Application {
public:
    /**
     * @brief Конструктор.
     * @param httpPort Порт, на котором будет слушать HTTP-сервер.
     * 
     * Автоматически добавляет стандартные сборщики: CPU, RAM, Disk, Temp, Process.
     */
    Application(int httpPort = 1234);
    
    ~Application();

    /**
     * @brief Запускает приложение (блокирующий вызов).
     * 
     * Запускает потоки сборщиков, HTTP-сервер и ждёт ввода символа из stdin.
     * После получения символа (обычно Enter) вызывает stop().
     */
    void run();

    /**
     * @brief Останавливает все потоки и сервер.
     * 
     * Устанавливает флаг m_running в false, вызывает stop() у сервера,
     * и дожидается завершения всех потоков сборщиков.
     */
    void stop();

private:
    SystemStats m_stats;                                 ///< Хранилище метрик
    HttpServer m_server;                                 ///< HTTP-сервер, ссылается на m_stats
    std::vector<std::unique_ptr<ICollector>> m_collectors; ///< Все сборщики
    std::vector<std::thread> m_threads;                  ///< Потоки для каждого сборщика
    std::atomic<bool> m_running;                         ///< Флаг работы потоков

    void addCollector(std::unique_ptr<ICollector> collector);
    void collectorLoop(ICollector* collector);           ///< Функция потока сборщика
};