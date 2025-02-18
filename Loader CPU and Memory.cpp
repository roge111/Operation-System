


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
#include <windows.h>
#include <utility> // Для std::pair
#include <atomic>


struct CPUUsage {
    ULARGE_INTEGER idleTime, kernelTime, userTime;
};
double avgUser = 0.0, avgSys = 0.0, avgWait = 0.0;
CPUUsage getCPUUsage() {
    CPUUsage usage;
    FILETIME idle, kernel, user;
    GetSystemTimes(&idle, &kernel, &user);
    usage.idleTime.QuadPart = reinterpret_cast<ULARGE_INTEGER*>(&idle)->QuadPart;
    usage.kernelTime.QuadPart = reinterpret_cast<ULARGE_INTEGER*>(&kernel)->QuadPart;
    usage.userTime.QuadPart = reinterpret_cast<ULARGE_INTEGER*>(&user)->QuadPart;
    return usage;
}

void printCPUUsage(const CPUUsage& before, const CPUUsage& after) {
    ULONGLONG idleDiff = after.idleTime.QuadPart - before.idleTime.QuadPart;
    ULONGLONG kernelDiff = after.kernelTime.QuadPart - before.kernelTime.QuadPart;
    ULONGLONG userDiff = after.userTime.QuadPart - before.userTime.QuadPart;
    ULONGLONG totalDiff = kernelDiff + userDiff;

    double userPercent = static_cast<double>(userDiff) / totalDiff * 100.0;
    double systemPercent = static_cast<double>(kernelDiff - idleDiff) / totalDiff * 100.0;
    double waitPercent = static_cast<double>(idleDiff) / totalDiff * 100.0;

    avgUser += userPercent;
    avgSys += systemPercent;
    avgWait += waitPercent;
}

void monitorCPU(bool& running) {
    CPUUsage before = getCPUUsage();
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CPUUsage after = getCPUUsage();
        printCPUUsage(before, after);
        before = after;
    }
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

double startLoaderCPU(int countThreads) {
    const int SIZE = 1000;
    int arr[SIZE];
    std::generate(arr, arr + SIZE, []() { return std::rand(); });

    auto start = std::chrono::high_resolution_clock::now();
    std::thread sortingThread(mergeSort, arr, 0, SIZE - 1, countThreads);
    sortingThread.join();  // Дожидаемся завершения сортировки
    auto end = std::chrono::high_resolution_clock::now();

    return std::chrono::duration<double, std::milli>(end - start).count(); // Возвращаем время в миллисекундах
}

const size_t BLOCK_SIZE = 1024 * 1024;
const size_t FILE_SIZE = static_cast<size_t>(1024) * 1024 * 1024; // Размер файла 1 ГБ

 randomReadTest(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) return 0.0;

    std::vector<char> buffer(BLOCK_SIZE);
    ifs.seekg(std::rand() % (FILE_SIZE - BLOCK_SIZE));

    auto start = std::chrono::high_resolution_clock::now();
    
    ifs.read(buffer.data(), BLOCK_SIZE);
    
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}


const size_t NUM_READS = 1000; // Количество случайных чтений

void createTestFile(const std::string& filename) {

    std::cout << "Start create file.";
     std::ofstream ofs(filename, std::ios::binary);
    std::vector<char> buffer(BLOCK_SIZE);

    // Инициализация генератора случайных чисел
    std::srand(static_cast<unsigned int>(std::time(0)));

    size_t totalWritten = 0;

    // Заполнение файла данными


    while (totalWritten < FILE_SIZE) {
        // Генерация случайных данных для буфера
        for (size_t i = 0; i < BLOCK_SIZE; ++i) {
            buffer[i] = static_cast<char>(std::rand() % 256); // Генерация случайного байта
        }

        // Запись буфера в файл
        ofs.write(buffer.data(), BLOCK_SIZE);
        totalWritten += BLOCK_SIZE;
    }

    ofs.close();
    std::cout << "\n";
    std::cout << "File " << filename << " created (1 GB)." << std::endl;

}

int main(int argc, char* argv[]) {
    
    std::string filename = "memoryLoader.dat";
    
    int countThreadsMemory = std::stoi(argv[1]);
    int countThreadsCPU = std::stoi(argv[2]);

        if (!outFile) {
            std::cerr << "Cannot open file!" << std::endl;
            return 1; // Завершаем программу с ошибкой
        }

        
        double result = 0.0;
        
        
        
            double delayTimeMemory = 0.0;
            double delayTimeCPU = 0.0;
            
            
            bool running = true;
            std::thread monitorThread(monitorCPU, std::ref(running));

            
            std::vector<std::thread> threads;
            std::vector<double> times(countThreadsMemory);
            if (countThreadsMemory > 0){
                
                
                for (int j = 0; j < countThreadsMemory; j++) {
                    threads.emplace_back([&times, j, &filename]() {
                        times[j] = randomReadTest(filename);
                    });
                }
                
            }

            if (countThreadsCPU > 0) delayTimeCPU += startLoaderCPU(countThreadsCPU);

            for (auto& th : threads) th.join();

            for (const auto& time : times) delayTimeMemory += time;
            if (countThreadsMemory > 0) {
                result += delayTimeMemory/countThreadsMemory + delayTimeCPU;
            } else {
                result += delayTimeCPU;
            }
            

            running = false;
            monitorThread.join();
            
        std::cout << countThreadsMemory << " " << countThreadsCPU << " "<< result << " " << avgUser << " " << avgSys << " " <<  avgWait << std::endl;
        
    


    return 0;
}
