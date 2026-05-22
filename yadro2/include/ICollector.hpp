/**
 * @file ICollector.hpp
 * @brief Интерфейс для всех сборщиков системных метрик.
 */

#pragma once
#include "SystemStats.hpp"

/**
 * @class ICollector
 * @brief Абстрактный класс, определяющий контракт сборщика данных.
 * 
 * Каждый сборщик (CPU, RAM, диски, температура, процессы) должен реализовать
 * метод update(), который обновляет переданный объект SystemStats, и метод
 * intervalMs(), возвращающий период опроса в миллисекундах.
 */
class ICollector 
{
public:
    virtual ~ICollector() = default;

    /**
     * @brief Обновляет статистику системы.
     * @param stats Ссылка на объект SystemStats, который будет заполнен новыми данными.
     * 
     * Этот метод вызывается в отдельном потоке с интервалом, заданным intervalMs().
     * Реализация должна быть потокобезопасной по отношению к stats (внутри stats уже есть мьютекс).
     */
    virtual void update(SystemStats& stats) = 0;

    /**
     * @brief Возвращает желаемый интервал опроса.
     * @return Интервал в миллисекундах.
     * 
     * Значение возвращается константой, чтобы Application мог планировать вызовы update().
     */
    virtual int intervalMs() const = 0;
};