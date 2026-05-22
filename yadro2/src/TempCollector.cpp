/**
 * @file TempCollector.cpp
 * @brief Реализация сборщика температуры CPU.
 */

#include "TempCollector.hpp"
#include <fstream>
#include <dirent.h>

using namespace std;

/**
 * @brief Получает температуру CPU в градусах Цельсия.
 * @return Температура или -1.0 при ошибке.
 * 
 * Ищет thermal_zone с типом, содержащим "x86", "cpu" или "acpitz".
 * Если не находит, использует thermal_zone0/temp.
 */
double TempCollector::getCPUTemperature() {
    DIR* dir = opendir("/sys/class/thermal");
    if (!dir) return -1.0;

    string temp_path;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        if (name.find("thermal_zone") == 0) {
            string type_path = "/sys/class/thermal/" + name + "/type";
            ifstream type_file(type_path);
            if (type_file.is_open()) {
                string type;
                getline(type_file, type);
                if (type.find("x86") != string::npos ||
                    type.find("cpu") != string::npos ||
                    type.find("acpitz") != string::npos) {
                    temp_path = "/sys/class/thermal/" + name + "/temp";
                    break;
                }
            }
        }
    }
    closedir(dir);

    if (temp_path.empty()) {
        temp_path = "/sys/class/thermal/thermal_zone0/temp";
    }

    ifstream file(temp_path);
    if (!file.is_open()) return -1.0;
    int temp_millidegree;
    file >> temp_millidegree;
    if (temp_millidegree <= 0) return -1.0;
    return static_cast<double>(temp_millidegree) / 1000.0;
}

/**
 * @brief Обновляет статистику температуры в SystemStats.
 * @param stats Целевой объект.
 */
void TempCollector::update(SystemStats& stats) {
    double temp = getCPUTemperature();
    if (temp > 0) {
        stats.setCPUTemp(temp);
    }
}