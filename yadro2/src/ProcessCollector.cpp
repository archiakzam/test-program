/**
 * @file ProcessCollector.cpp
 * @brief Реализация сборщика процессов.
 */

#include "ProcessCollector.hpp"
#include <fstream>
#include <sstream>
#include<iostream>
#include <dirent.h>
#include <cstring>
#include <algorithm>
#include <unistd.h>

using namespace std;

/**
 * @brief Суммарное время CPU системы из /proc/stat (строка "cpu").
 * @return Общее время в тиках.
 */
long long ProcessCollector::getTotalCpuTime() {
    ifstream file("/proc/stat");
    if (!file.is_open()) return 0;
    string line;
    getline(file, line);
    if (line.find("cpu ") != 0) return 0;
    istringstream ss(line.substr(5));
    long long user, nice, system, idle, iowait, irq, softirq, steal;
    ss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    return user + nice + system + idle + iowait + irq + softirq + steal;
}

/**
 * @brief Количество ядер CPU (число строк cpu0, cpu1… в /proc/stat).
 * @return Число ядер.
 */
int ProcessCollector::getNumCpus() {
    static int cached = 0;
    if (cached > 0) return cached;
    ifstream file("/proc/stat");
    string line;
    int count = 0;
    while (getline(file, line)) {
        if (line.compare(0, 3, "cpu") == 0 && line[3] != ' ')
            count++;
    }
    cached = count > 0 ? count : 1;
    return cached;
}

/**
 * @brief Получить список процессов с их CPU-временем и памятью.
 * @param[out] current_cpu_times Карта pid -> utime+stime (тики).
 * @param[out] total_cpu_time Суммарное время CPU системы.
 * @return Вектор ProcessInfo (пока без cpu_percent).
 */
vector<ProcessInfo> ProcessCollector::getProcessList
(map<int, long long>& current_cpu_times,long long& total_cpu_time)
{
    vector<ProcessInfo> result;
    DIR* dir = opendir("/proc");
    if (!dir) {
        cerr << "Failed to open /proc\n";
        return result;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        char* endptr;
        long pid = strtol(entry->d_name, &endptr, 10);
        if (*endptr != '\0') continue;

        string stat_path = "/proc/" + string(entry->d_name) + "/stat";
        ifstream stat_file(stat_path);
        if (!stat_file.is_open()) continue;

        string line;
        getline(stat_file, line);
        if (line.empty()) continue;

        size_t lparen = line.find('(');
        size_t rparen = line.rfind(')');
        if (lparen == string::npos || rparen == string::npos || rparen < lparen) continue;
        string name = line.substr(lparen + 1, rparen - lparen - 1);

        string after_name = line.substr(rparen + 2);
        istringstream iss(after_name);
        int field_num = 0;
        long long utime = 0, stime = 0;
        string token;
        while (iss >> token && field_num <= 15) {
            if (field_num == 13) utime = stoll(token);
            else if (field_num == 14) stime = stoll(token);
            field_num++;
        }
        long long proc_cpu_time = utime + stime;
        current_cpu_times[pid] = proc_cpu_time;

        string status_path = "/proc/" + string(entry->d_name) + "/status";
        ifstream status_file(status_path);
        long memory_kb = 0;
        if (status_file.is_open()) {
            string line2;
            while (getline(status_file, line2)) {
                if (line2.compare(0, 6, "VmRSS:") == 0) {
                    istringstream ss2(line2.substr(6));
                    ss2 >> memory_kb;
                    break;
                }
            }
        }
        ProcessInfo info;
        info.pid = static_cast<int>(pid);
        info.name = name;
        info.memory_mb = memory_kb / 1024;
        info.cpu_percent = 0.0; 
        result.push_back(info);
    }
    closedir(dir);
    total_cpu_time = getTotalCpuTime();
    return result;
}

/**
 * @brief Обновляет статистику процессов.
 * @param stats Целевой объект.
 * 
 * Алгоритм:
 * - Получает текущий список процессов и их CPU-времена.
 * - Сравнивает с предыдущими значениями (хранятся в prev_cpu_times, prev_total_cpu).
 * - Для каждого процесса вычисляет CPU%: (proc_delta / total_delta) * 100 / num_cpus.
 * - Сортирует процессы по убыванию CPU%.
 */
void ProcessCollector::update(SystemStats& stats) {
    if (num_cpus == 0) num_cpus = getNumCpus();
    
    map<int, long long> curr_cpu_times;
    long long curr_total_cpu = 0;
    auto processes = getProcessList(curr_cpu_times, curr_total_cpu);
    
    if (prev_total_cpu == 0 || prev_cpu_times.empty()) {
        prev_cpu_times = move(curr_cpu_times);
        prev_total_cpu = curr_total_cpu;
        stats.setProcesses(processes);
        return;
    }
    
    long long total_delta = curr_total_cpu - prev_total_cpu;
    if (total_delta <= 0) {
        prev_cpu_times = move(curr_cpu_times);
        prev_total_cpu = curr_total_cpu;
        stats.setProcesses(processes);
        return;
    }
    
    for (auto& proc : processes) {
        auto it = prev_cpu_times.find(proc.pid);
        if (it != prev_cpu_times.end()) {
            long long proc_delta = curr_cpu_times[proc.pid] - it->second;
            if (proc_delta > 0) {
                double cpu_percent = (static_cast<double>(proc_delta) / total_delta) * 100.0;
                cpu_percent = cpu_percent / num_cpus;
                if (cpu_percent > 100.0) cpu_percent = 100.0;
                proc.cpu_percent = cpu_percent;
            }
        }
    }
    
    prev_cpu_times = move(curr_cpu_times);
    prev_total_cpu = curr_total_cpu;
    
    sort(processes.begin(), processes.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
        return a.cpu_percent > b.cpu_percent;
    });
    if (num_cpus == 0) num_cpus = getNumCpus();
    
    stats.setProcesses(processes);
}