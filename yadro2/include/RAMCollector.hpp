/**
 * @file RAMCollector.hpp
 * @brief Сборщик использования оперативной памяти.
 */

#pragma once
#include "ICollector.hpp"

/**
 * @class RAMCollector
 * @brief Читает /proc/meminfo и вычисляет процент занятой RAM.
 * 
 * Использует строки MemTotal и MemAvailable. Формула:
 * usage = (MemTotal - MemAvailable) / MemTotal * 100%.
 * Интервал опроса — 1 секунда.
 */
class RAMCollector : public ICollector {
public:
    void update(SystemStats& stats) override;
    int intervalMs() const override { return 1000; }

private:
    /**
     * @brief Получает процент использования RAM.
     * @return Значение от 0 до 100, или -1.0 при ошибке.
     */
    double getRAMUsage();
};