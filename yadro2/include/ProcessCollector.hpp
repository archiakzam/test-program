/**
 * @file ProcessCollector.hpp
 * @brief Сборщик информации о процессах.
 */

#pragma once

#include "ICollector.hpp"
#include "SystemStats.hpp"
#include <map>
#include <vector>
/**
 * @class ProcessCollector
 * @brief Собирает список процессов и вычисляет их CPU%.
 * 
 * Читает /proc/[pid]/stat и /status. Для расчёта CPU% хранит предыдущие значения
 * общего времени системы и времени каждого процесса.
 */
class ProcessCollector : public ICollector {
public:
    void update(SystemStats& stats) override;
    int intervalMs() const override { return 2000; }
    
private:
    std::vector<ProcessInfo> getProcessList(std::map<int, long long>& current_cpu_times, long long& total_cpu_time);
    static long long getTotalCpuTime();
    static int getNumCpus();
    
    std::map<int, long long> prev_cpu_times;
    long long prev_total_cpu = 0;
    int num_cpus = 0;
};