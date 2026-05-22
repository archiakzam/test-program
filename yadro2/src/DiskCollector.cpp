/**
 * @file DiskCollector.cpp
 * @brief Реализация сборщика информации о дисках.
 */

#include "DiskCollector.hpp"
#include <fstream>
#include <sstream>
#include <sys/statvfs.h>

using namespace std;

/**
 * @brief Проверяет, является ли файловая система «реальной».
 * @param fstype Тип файловой системы (строка из /proc/mounts).
 * @return true, если ФС не из списка виртуальных.
 */
bool DiskCollector::isRealFilesystem(const string& fstype) {
    static const vector<string> virtual_fs = {
        "tmpfs", "devtmpfs", "proc", "sysfs", "cgroup", "cgroup2",
        "devpts", "securityfs", "pstore", "bpf", "autofs", "tracefs",
        "squashfs", "overlay", "fuse", "fusectl", "efivarfs"
    };
    for (const auto& vfs : virtual_fs) {
        if (fstype == vfs) return false;
    }
    return true;
}

/**
 * @brief Получает список всех реальных дисковых разделов с процентом заполнения.
 * @return Вектор структур DiskInfo.
 * 
 * Читает /proc/mounts, фильтрует виртуальные ФС и специальные каталоги (/proc, /sys, /dev),
 * для каждой точки монтирования вызывает statvfs и вычисляет used/total*100.
 */
vector<DiskInfo> DiskCollector::getDiskUsage() {
    vector<DiskInfo> result;
    ifstream mounts("/proc/mounts");
    if (!mounts.is_open()) return result;

    string line;
    while (getline(mounts, line)) {
        istringstream ss(line);
        string dev, mount_point, fstype, opts;
        int dump, pass;
        ss >> dev >> mount_point >> fstype >> opts >> dump >> pass;

        if (!isRealFilesystem(fstype)) continue;
        if (mount_point.find("/proc") == 0 ||
            mount_point.find("/sys") == 0 ||
            mount_point.find("/dev") == 0) continue;

        struct statvfs stat;
        if (statvfs(mount_point.c_str(), &stat) != 0) continue;

        unsigned long long total = stat.f_blocks * stat.f_frsize;
        unsigned long long free = stat.f_bfree * stat.f_frsize;
        unsigned long long used = total - free;
        if (total == 0) continue;
        double percent = 100.0 * used / total;

        result.push_back({ mount_point, percent });
    }
    return result;
}

/**
 * @brief Обновляет статистику дисков в SystemStats.
 * @param stats Целевой объект.
 */
void DiskCollector::update(SystemStats& stats) {
    auto disks = getDiskUsage();
    stats.setDisks(disks);
}