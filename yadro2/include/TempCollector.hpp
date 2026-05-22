/**
 * @file TempCollector.hpp
 * @brief Сборщик температуры CPU.
 */

#pragma once
#include "ICollector.hpp"

/**
 * @class TempCollector
 * @brief Читает температуру из /sys/class/thermal.
 * 
 * Ищет thermal_zone с типом, содержащим "x86", "cpu" или "acpitz",
 * при отсутствии использует thermal_zone0. Читает файл temp (миллиградусы),
 * переводит в градусы Цельсия. Интервал опроса — 2 секунды.
 */
class TempCollector : public ICollector {
public:
    void update(SystemStats& stats) override;
    int intervalMs() const override { return 2000; }

private:
    /**
     * @brief Получает температуру CPU в градусах Цельсия.
     * @return Температура, или -1.0 при ошибке.
     */
    double getCPUTemperature();
};