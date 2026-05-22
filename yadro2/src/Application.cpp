/**
 * @file Application.cpp
 * @brief Реализация главного класса приложения.
 */

#include "Application.hpp"
#include "CPUCollector.hpp"
#include "RAMCollector.hpp"
#include "DiskCollector.hpp"
#include "TempCollector.hpp"
#include "ProcessCollector.hpp"
#include "ICollector.hpp"
#include <iostream>

using namespace std;

Application::Application(int httpPort)
    : m_server(m_stats, httpPort), m_running(false) {
    addCollector(make_unique<CPUCollector>());
    addCollector(make_unique<RAMCollector>());
    addCollector(make_unique<DiskCollector>());
    addCollector(make_unique<TempCollector>());
    addCollector(make_unique<ProcessCollector>());
}

Application::~Application() {
    stop();
}

void Application::addCollector(unique_ptr<ICollector> collector) {
    m_collectors.push_back(move(collector));
}

/**
 * @brief Функция, выполняемая в потоке каждого сборщика.
 * @param collector Указатель на сборщик, который нужно вызывать.
 * 
 * В цикле, пока приложение запущено, вызывает update() сборщика,
 * замеряет время выполнения и засыпает на оставшееся время интервала.
 */
void Application::collectorLoop(ICollector* collector) {
    while (m_running) {
        auto start = chrono::steady_clock::now();
        collector->update(m_stats);
        auto elapsed = chrono::steady_clock::now() - start;
        int interval = collector->intervalMs();
        int sleep_ms = interval - chrono::duration_cast<chrono::milliseconds>(elapsed).count();
        if (sleep_ms > 0) {
            this_thread::sleep_for(chrono::milliseconds(sleep_ms));
        }
    }
}

void Application::run() {
    m_running = true;
    for (auto& collector : m_collectors) {
        m_threads.emplace_back(&Application::collectorLoop, this, collector.get());
    }
    m_server.start();
    cin.get();
    stop();
}

void Application::stop() {
    m_running = false;
    m_server.stop();
    for (auto& t : m_threads) {
        if (t.joinable()) t.join();
    }
    m_threads.clear();
}