/**
 * @file CPUCollector.cpp
 * @brief Реализация сборщика загрузки CPU.
 */

#include "CPUCollector.hpp"
#include <fstream>
#include <sstream>

using namespace std;

/**
 * @brief Считывает сырые счётчики CPU из /proc/stat.
 * @param data Структура для заполнения.
 * @return true если чтение успешно, иначе false.
 * 
 * Читает первую строку файла /proc/stat, начинающуюся с "cpu ".
 * Формат строки описан в man proc(5).
 */
bool CPUCollector::readCPUData(CPUData& data) {
    ifstream file("/proc/stat");
    if (!file.is_open()) return false;
    string line;
    getline(file, line);
    if (line.find("cpu ") != 0) return false;
    istringstream ss(line.substr(5));
    ss >> data.user >> data.nice >> data.system >> data.idle
        >> data.iowait >> data.irq >> data.softirq >> data.steal;
    return true;
}
/**
 * @brief Вычисляет процент использования CPU за период.
 * @param prev Предыдущие значения счётчиков.
 * @param curr Текущие значения.
 * @return Загрузка в процентах (0–100).
 * 
 * Используется формула: (общее время - idle) / общее время * 100.
 * idle включает iowait.
 */
double CPUCollector::calculateUsage(const CPUData& prev, const CPUData& curr) {
    unsigned long long prev_total = prev.user + prev.nice + prev.system + prev.idle +
        prev.iowait + prev.irq + prev.softirq + prev.steal;
    unsigned long long curr_total = curr.user + curr.nice + curr.system + curr.idle +
        curr.iowait + curr.irq + curr.softirq + curr.steal;
    unsigned long long prev_idle = prev.idle + prev.iowait;
    unsigned long long curr_idle = curr.idle + curr.iowait;

    unsigned long long total_delta = curr_total - prev_total;
    unsigned long long idle_delta = curr_idle - prev_idle;

    if (total_delta == 0) return 0.0;
    return 100.0 * (1.0 - static_cast<double>(idle_delta) / total_delta);
}
/**
 * @brief Обновляет статистику CPU.
 * @param stats Объект SystemStats, куда записывается результат.
 * 
 * Вызывается каждый интервал (1000 мс). При первом запуске сохраняет начальные
 * показания без вычисления процента. В следующих вызовах считает usage и вызывает stats.setCPU().
 */
void CPUCollector::update(SystemStats& stats) {
    CPUData curr;
    if (!readCPUData(curr)) return;
    if (m_prev.user == 0 && m_prev.nice == 0 && m_prev.system == 0 && m_prev.idle == 0) {
        m_prev = curr;
        return;
    }
    double usage = calculateUsage(m_prev, curr);
    m_prev = curr;
    stats.setCPU(usage);
}