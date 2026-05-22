/**
 * @file DiskCollector.hpp
 * @brief Сборщик информации о дисках.
 */

#pragma once
#include "ICollector.hpp"
#include <vector>

/**
 * @class DiskCollector
 * @brief Собирает процент заполнения для всех реальных файловых систем.
 * 
 * Анализирует /proc/mounts, фильтрует виртуальные ФС (proc, sysfs, tmpfs и т.д.),
 * для каждой точки монтирования вызывает statvfs и вычисляет usagePercent.
 * Интервал опроса — 2 секунды.
 */
class DiskCollector : public ICollector {
public:
    void update(SystemStats& stats) override;
    int intervalMs() const override { return 2000; }

private:
    /**
     * @brief Получает список всех дисков с заполненностью.
     * @return Вектор структур DiskInfo.
     */
    std::vector<DiskInfo> getDiskUsage();

    /**
     * @brief Проверяет, является ли файловая система "реальной" (не виртуальной).
     * @param fstype Тип файловой системы (например, "ext4", "tmpfs").
     * @return true если реальная (подлежит мониторингу), false если виртуальная.
     */
    bool isRealFilesystem(const std::string& fstype);
};