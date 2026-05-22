/**
 * @file SystemStats.cpp
 * @brief Реализация потокобезопасного хранилища метрик.
 */

#include "SystemStats.hpp"

using namespace std;

void SystemStats::setCPU(double usage) {
    lock_guard<mutex> lock(m_mutex);
    m_cpu = usage;
}

void SystemStats::setRAM(double usage) {
    lock_guard<mutex> lock(m_mutex);
    m_ram = usage;
}

void SystemStats::setCPUTemp(double temp) {
    lock_guard<mutex> lock(m_mutex);
    m_cpu_temp = temp;
}

void SystemStats::setDisks(const vector<DiskInfo>& disks) {
    lock_guard<mutex> lock(m_mutex);
    m_disks = disks;
}

void SystemStats::setProcesses(const vector<ProcessInfo>& procs) {
    lock_guard<mutex> lock(m_mutex);
    m_processes = procs;
}

double SystemStats::getCPU() const {
    lock_guard<mutex> lock(m_mutex);
    return m_cpu;
}

double SystemStats::getRAM() const {
    lock_guard<mutex> lock(m_mutex);
    return m_ram;
}

double SystemStats::getCPUTemp() const {
    lock_guard<mutex> lock(m_mutex);
    return m_cpu_temp;
}

vector<DiskInfo> SystemStats::getDisks() const {
    lock_guard<mutex> lock(m_mutex);
    return m_disks;
}

vector<ProcessInfo> SystemStats::getProcesses() const {
    lock_guard<mutex> lock(m_mutex);
    return m_processes;
}