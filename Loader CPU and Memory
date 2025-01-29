// Сделать время измерения отдельно для сортировки, отдельно для чтения. (Исправлено)
// Создавать отдельно потоки для сортировки, отдельно для памяти (Исправлено)
// Реализовать возможность запуска их одновременно (Исправлено)
//Парметры WAIT%, USER%, SYS%  - измерять сначало зависимость от изменения потоков на CPU, потом на Memory (Исправлено)
// Нет смысла создавать большое количество потоков на компьютер с 12-ю логическими процессами.


//Код был сделан более читабельным, в меру оптимизирован с добавление заметок выше
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <thread>
#include <cstdlib>
#include <mutex>
#include <sstream>
#include <string>
#include <unistd.h>

struct CPUUsage {
    long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
};

CPUUsage getCPUUsage() {
    CPUUsage cpuUsage;
    std::ifstream file("/proc/stat");
    std::string line;
    if (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string cpu;
        ss >> cpu >> cpuUsage.user >> cpuUsage.nice >> cpuUsage.system >> cpuUsage.idle
           >> cpuUsage.iowait >> cpuUsage.irq >> cpuUsage.softirq >> cpuUsage.steal
           >> cpuUsage.guest >> cpuUsage.guest_nice;
    }
    return cpuUsage;
}

void printCPUUsage(const CPUUsage& before, const CPUUsage& after) {
    long totalBefore = before.user + before.nice + before.system + before.idle + before.iowait + before.irq + before.softirq + before.steal;
    long totalAfter = after.user + after.nice + after.system + after.idle + after.iowait + after.irq + after.softirq + after.steal;

    long totalDiff = totalAfter - totalBefore;
    long userDiff = after.user - before.user;
    long systemDiff = after.system - before.system;
    long waitDiff = after.iowait - before.iowait;

    double userPercent = (static_cast<double>(userDiff) / totalDiff) * 100.0;
    double systemPercent = (static_cast<double>(systemDiff) / totalDiff) * 100.0;
    double waitPercent = (static_cast<double>(waitDiff) / totalDiff) * 100.0;

    std::cout << "USER%: " << userPercent << "%, SYS%: " << systemPercent << "%, WAIT%: " << waitPercent << "%" << std::endl;
}


//Сортировка слиянием
void merge(int arr[], int left, int mid, int right) {
    int n1 = mid - left + 1, n2 = right - mid;
    std::vector<int> L(arr + left, arr + left + n1);
    std::vector<int> R(arr + mid + 1, arr + mid + 1 + n2);

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

//Запуск сортировки слияением с распределнием на потоки
void mergeSort(int arr[], int left, int right, int maxThreads) {
   if (left < right) {
        int mid = left + (right - left) / 2;

        // Определяем количество доступных потоков
        if (maxThreads > 1) {
            std::thread leftThread(mergeSort, arr, left, mid, maxThreads / 2);
            std::thread rightThread(mergeSort, arr, mid + 1, right, maxThreads / 2);

            leftThread.join();
            rightThread.join();
        } else {
            mergeSort(arr, left, mid, 1);
            mergeSort(arr, mid + 1, right, 1);
        }

        // Сливаем отсортированные половины
        merge(arr, left, mid, right);
    }
}

void startLoaderCPU(int countThreads) {
    const int SIZE = 1000;
    int arr[SIZE];
    std::generate(arr, arr + SIZE, []() { return std::rand(); });

    auto start = std::chrono::high_resolution_clock::now();
    std::thread sortingThread(mergeSort, arr, 0, SIZE - 1, countThreads);
    sortingThread.join();  // Дожидаемся завершения сортировки
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Delay time for sorting: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";
}

const size_t BLOCK_SIZE = 1024 * 1024;
void randomReadTest(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::binary);
    std::vector<char> buffer(BLOCK_SIZE);

    auto start = std::chrono::high_resolution_clock::now();
    ifs.seekg(std::rand() % 100);
    ifs.read(buffer.data(), BLOCK_SIZE);
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Delay time for random reading: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";
}

int main() {
    std::string filename = "memoryLoader.dat";
    int createFile;
    std::cout << "Create a new file? (1 (Y)/ 0 (N)) >> ";
    std::cin >> createFile;
    if (createFile == 1) {
        std::ofstream ofs(filename, std::ios::binary);
        std::vector<char> buffer(BLOCK_SIZE, 'A');
        for (size_t i = 0; i < 1024; ++i) ofs.write(buffer.data(), BLOCK_SIZE);
        std::cout << "File created.\n";
    }

    int countThreadsMemory, countThreadsCPU;
    std::cout << "Enter the number of threads for Memory: ";
    std::cin >> countThreadsMemory;
    std::cout << "Enter the number of threads for CPU: ";
    std::cin >> countThreadsCPU;

    CPUUsage before = getCPUUsage();

    std::vector<std::thread> threads;
    for (int i = 0; i < countThreadsMemory; i++) threads.emplace_back(randomReadTest, filename);
    if (countThreadsCPU > 0) startLoaderCPU(countThreadsCPU);

    for (auto& th : threads) th.join();

    CPUUsage after = getCPUUsage();
    printCPUUsage(before, after);

    return 0;
}
