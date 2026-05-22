/**
 * @file CPUCollector.hpp
 * @brief Сборщик загрузки процессора.
 */

#pragma once
#include "ICollector.hpp"

/**
 * @class CPUCollector
 * @brief Реализация сборщика CPU через /proc/stat.
 * 
 * Каждую секунду читает счётчики времени CPU из /proc/stat,
 * вычисляет процент использования за интервал и сохраняет в SystemStats.
 */
class CPUCollector : public ICollector {
public:
    void update(SystemStats& stats) override;
    int intervalMs() const override { return 1000; }

private:
    /**
     * @struct CPUData
     * @brief Сырые значения счётчиков CPU из /proc/stat.
     */
    struct CPUData {
        unsigned long long user;     ///< время пользовательских процессов
        unsigned long long nice;     ///< время с nice
        unsigned long long system;   ///< время ядра
        unsigned long long idle;     ///< время простоя
        unsigned long long iowait;   ///< время ожидания ввода-вывода
        unsigned long long irq;      ///< время аппаратных прерываний
        unsigned long long softirq;  ///< время программных прерываний
        unsigned long long steal;    ///< время, украденное виртуализацией
    };

    CPUData m_prev;                  ///< предыдущие значения для расчёта дельты

    /**
     * @brief Считывает счётчики CPU из /proc/stat.
     * @param data Структура для заполнения.
     * @return true при успехе, false при ошибке.
     */
    bool readCPUData(CPUData& data);

    /**
     * @brief Вычисляет процент использования CPU между двумя замерами.
     * @param prev Предыдущие значения.
     * @param curr Текущие значения.
     * @return Процент загрузки (0.0–100.0).
     */
    double calculateUsage(const CPUData& prev, const CPUData& curr);
};