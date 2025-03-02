


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
void bubbleSort(std::vector<int>& sublist){
    int n = sublist.size();
    for (int i = 0; i < n - 1; ++i){
        for (int j = 0; j < n - i - 1; ++j ){
            if (sublist[j] >  sublist[j + 1]){
                std::swap(sublist[j], sublist[j + 1]);
            }
        }
    }
}


std::vector<std::vector<int>> splitList(std::vector<int>& data, int count_threads){

    std::vector<std::vector<int>> sublist (count_threads);

    int split_size = data.size()/count_threads;

    int remainder = data.size() % count_threads;
    int start = 0;

    for (int i = 0; i < count_threads; ++i){
        int end = start + split_size + (i < remainder ? 1 : 0);
        sublist[i].assign(data.begin() + start, data.begin() + end);
        start = end;
    }
    return sublist;


}


// Функция, которая объединяет отсортированные подсписки в один отсортированный список
std::vector<int> merge_sorted_sublists(const std::vector<std::vector<int>>& sorted_sublists) {
    std::vector<int> result;
    for (const auto& sublist : sorted_sublists) {
        result.insert(result.end(), sublist.begin(), sublist.end());
    }
    std::sort(result.begin(), result.end());
    return result;
}

double startLoaderCPU(int count_threads){
    std::vector<int> list;
    for (int i = 0; i < 100000; ++i) {
        //std::cout << i + 1 << "\n";
        list.push_back(rand() % 1000);
    }

    std::vector<std::vector<int>> sublists = splitList(list, count_threads);

    std::vector<std::thread> threads;

    auto start = std::chrono::high_resolution_clock::now();

    for (auto& sublist : sublists){
        threads.emplace_back(bubbleSort, std::ref(sublist));
    }

    for (auto& thread : threads){
        thread.join();
    }

    std::vector<int> sort_list = merge_sorted_sublists(sublists);


    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();

    
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
