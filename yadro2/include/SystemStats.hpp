/**
 * @file SystemStats.hpp
 * @brief Потокобезопасное хранилище всех собираемых метрик.
 */

#pragma once
#include <string>
#include <vector>
#include <mutex>

/**
 * @struct DiskInfo
 * @brief Информация о дисковом разделе.
 */
struct DiskInfo
{
    std::string mountPoint;   ///< Точка монтирования (например, "/", "/home")
    double usagePercent;      ///< Процент занятого места (0.0–100.0)
};

/**
 * @struct ProcessInfo
 * @brief Информация о процессе.
 */
struct ProcessInfo {
    int pid;                  ///< Идентификатор процесса
    std::string name;         ///< Имя исполняемого файла
    double cpu_percent;       ///< Загрузка CPU в процентах (от 0 до 100*число ядер, нормализовано)
    long memory_mb;           ///< Используемая физическая память в мегабайтах (VmRSS)
};

/**
 * @class SystemStats
 * @brief Потокобезопасный контейнер для метрик.
 * 
 * Все методы чтения и записи защищены мьютексом, поэтому объект можно безопасно
 * использовать из разных потоков сборщиков и HTTP-сервера.
 */
class SystemStats
{
public:
    // Setters (вызываются сборщиками)
    void setCPU(double usage);
    void setRAM(double usage);
    void setCPUTemp(double temp);
    void setDisks(const std::vector<DiskInfo>& disks);
    void setProcesses(const std::vector<ProcessInfo>& procs);
    
    // Getters (вызываются HTTP-сервером для формирования JSON)
    double getCPU() const;
    double getRAM() const;
    double getCPUTemp() const;
    std::vector<DiskInfo> getDisks() const;
    std::vector<ProcessInfo> getProcesses() const;

private:
    mutable std::mutex m_mutex;          ///< Защищает все нижеследующие поля
    double m_cpu = 0.0;
    double m_ram = 0.0;
    double m_cpu_temp = 0.0;
    std::vector<DiskInfo> m_disks;
    std::vector<ProcessInfo> m_processes;
};