/**
 * @file RAMCollector.cpp
 * @brief Реализация сборщика оперативной памяти.
 */

#include "RAMCollector.hpp"
#include <fstream>
#include <sstream>

using namespace std;

/**
 * @brief Получает процент использования RAM из /proc/meminfo.
 * @return Процент (0–100) или -1.0 при ошибке.
 * 
 * Используются строки MemTotal и MemAvailable.
 */
double RAMCollector::getRAMUsage() {
    ifstream file("/proc/meminfo");
    if (!file.is_open()) return -1.0;
    string line;
    long memTotal = 0, memAvailable = 0;
    while (getline(file, line)) {
        if (line.compare(0, 9, "MemTotal:") == 0) {
            istringstream ss(line.substr(9));
            ss >> memTotal;
        }
        else if (line.compare(0, 13, "MemAvailable:") == 0) {
            istringstream ss(line.substr(13));
            ss >> memAvailable;
            if (memTotal > 0) break;
        }
    }
    if (memTotal == 0) return -1.0;
    return 100.0 * (memTotal - memAvailable) / memTotal;
}

/**
 * @brief Обновляет статистику RAM в SystemStats.
 * @param stats Целевой объект.
 */
void RAMCollector::update(SystemStats& stats) {
    double usage = getRAMUsage();
    if (usage >= 0)
        stats.setRAM(usage);
}