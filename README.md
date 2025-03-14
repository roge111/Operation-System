

## Задание

![Задание](https://github.com/user-attachments/assets/962513e2-616e-4b5e-87fe-6de14678f686)

## Оболочка Shell

Оболочка Shell находиться в фалй main и выполняет две команды: echo и type в зависиммости от того, что напищшет пользователь.

        void runWindows(const char* command, int countThreadsMemory, int countThreadsCPU) {
                    STARTUPINFO si;
                    PROCESS_INFORMATION pi;
                
                    ZeroMemory(&si, sizeof(si));
                    si.cb = sizeof(si);
                    ZeroMemory(&pi, sizeof(pi));
                
                    
                
                    
                    auto start = std::chrono::high_resolution_clock::now();
                
                    std::string cmd = "cmd.exe /C " + std::string(command);
                
                    if (!CreateProcess("C:\\Windows\\System32\\cmd.exe", 
                            (LPSTR)cmd.c_str(),
                            NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                        std::cerr << "CreateProcess failed. Error: " << GetLastError() << "\n";
                    } else {
                        WaitForSingleObject(pi.hProcess, INFINITE);
                        CloseHandle(pi.hProcess);
                        CloseHandle(pi.hThread);
                        std::cout << "Successfully!\n";
                    }
                
                    auto end = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> elapsed_seconds = end - start;
                    std::cout << "Work time is " << elapsed_seconds.count() << " seconds\n";
                }

Запускает системную команду через CreateProcess используя комадную строку. Так же измеряет время работы.

## Нагрзучик памяти и CPU

Программа в файле LoaderCPUandMemory.cpp — это код, который предназначен для исследования того, как влияет количество запущенных нагрузчиков либо на чтение данных с дисковой памяти, либо на сортировку сгенерированного массива размером 1000.

Также идет измерение таких значений: USER%, SYS%, WAIT%.

USER% показывает, сколько времени CPU тратит на выполнение пользовательских процессов.
SYS% — сколько времени CPU проводит в режиме ядра.
WAIT% — сколько времени CPU простаивает в режиме ожидания ввода/вывода.

Программа подразумевает создание файла размером 1 Гб. Это сделано специально для того, чтобы нагрузить дисковую память и изменения в измерениях были заметными. 

Также есть возможность выбрать количество потоков для запуска нагрузчика на CPU и на память дисковую. Два цикла в main позволяют решить сразу несколько задач: запуск отдельно нагрузчика CPU и на дисковую память, одновременный запуск обоих нагрузчиков. 

Нагрузчик на память дисковую — randomReadtest. Здесь идет выбор рандомной позиции в файле, откуда идет считывание массива данных размера 1000 в программу. 

Нагрузчик на CPU — startLoaderCPU. Это лишь программа, которая реализует запуск сортировки. Сортировка заключается в том, что несколько нагрузчиков сортируют один массив. Для этого используется сортировка слиянием. Самый удобный метод для сортировки несколькими потоками.

# Исходный код на C++

Пройдемся по коду на C++ 

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

Указанных выше три метода выполняют получение системных метрик USER%, SYS%, WAIT%. Переменные _avgUser, avgSys и avgWait_ хранят эти метрики соответсвенно


Текущая функция bubbleSort реализует сортировку пузырьком. Это лишь часть сортировки. Сортировка идет такми образом, что указанное количесвто потоков сортируют один массив параллельно. 

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
Метод marge_sorted_sublists объединяет все отсоритированные куски массива (подмассивы) в один массив 

        std::vector<int> merge_sorted_sublists(const std::vector<std::vector<int>>& sorted_sublists) {
            std::vector<int> result;
            for (const auto& sublist : sorted_sublists) {
                result.insert(result.end(), sublist.begin(), sublist.end());
            }
            std::sort(result.begin(), result.end());
            return result;
        }

splitList - обратная функция. Нужна для разделения одного списка на подмассивы. Количесвто подмассивов - count_threads

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


randomReadTest - выполняет чтение файла объемом не менее 1Гб из диска. Сделано для того, чтобы измерять задержки чтения с дисковой памяти. Чтение из файла идет блоком BLOCK_SIZE из рандомной точки файла. За это отвечает _ifs.seekg(std::rand() % (FILE_SIZE - BLOCK_SIZE));_. А само чтение идет с помощью    _ifs.read(buffer.data(), BLOCK_SIZE);_

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
createTestFile — метод, который создает файл размером 1 Гб (установленного в переменной FILE_SIZE). Идет генерация случайных данных и запись. Функция используется по желанию пользователя. 

    int main(int argc, char* argv[]) {
    
    int countThreadsMemory = std::stoi(argv[1]);
    int countThreadsCPU = std::stoi(argv[2]);
    //std::cout << countThreadsMemory << countThreadsCPU << "\n";
   

        


        std::string filenameOut = "outputCPP.txt";
        

        std::ofstream outFile;
        outFile.open(filenameOut);

        if (!outFile) {
            std::cerr << "Cannot open file!" << std::endl;
            return 1; // Завершаем программу с ошибкой
        }

        


        
        // std::this_thread::sleep_for(std::chrono::seconds(30));
        
        double result = 0.0;
        // LONG contextSwitches = 0.0;
        
        
        
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
            // contextSwitches += getContextSwitches();

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

В методе _main_ идет чтение данных из файла «memoryLoader.dat». Если такого файла нет, следует в код добавить запуск метода createTestFile. Ранее полагалось, что будет идти запрос пользователю на создание файла с данными, но было убрано для интеграции с Python. Метод принимает два значения в наборе argv (набор из двух элементов). В этом наборе два значения в типе char* — countThreadsMemory и countThreadsCPU соответственно. Они отвечают за количество нагрузок на память дисковую и CPU. Нагрузчиком на память является randomReadTest, а на CPU — startLoaderCPU. Далее идет получение данных времени работы нагрузчиков на память и CPU в переменную result, result — это сумма delayTimeMemory и delayTimeCPU. Идет вывод всех данных. Но этот вывод перехватывается программой на Python, лежащей в файле PyAnalize.py. 


# Программа на Python

Программа лежит в файле PyAnalize.py. Код используется для записи результатов в csv-файл или любой другой, который укажет пользователь. Также Python используется для измерения количества переходов контекста в системе. Программа содержит набор параметров, состоящий из пар (число нагрузчиков на память, число нагрузчиков на CPU), которые потом передаются в программу на C++. Это сделано для того, чтобы «сгладить» резкие скачки измерений. Например, время работы может быть равно 1 мс, 1,5 мс и резко может быть 3 мс. Тогда берется среднее арифметическое из всех измерений. Далее данные записываются в файл для дальнейшего анализа в другой программе. Разберем программу по порядку. 

    class FILETIME(ctypes.Structure):
        _fields_ = [("dwLowDateTime", ctypes.c_uint),
                    ("dwHighDateTime", ctypes.c_uint)]
    
    def get_context_switches_per_second():
        kernel32 = ctypes.WinDLL('kernel32', use_last_error=True)
        kernel32.GetSystemTimes.argtypes = (ctypes.POINTER(FILETIME),
                                            ctypes.POINTER(FILETIME),
                                            ctypes.POINTER(FILETIME))
        kernel32.GetSystemTimes.restype = ctypes.c_bool
    
        idle_time_start = FILETIME()
        kernel32.GetSystemTimes(None, None, ctypes.byref(idle_time_start))
    
        time.sleep(1)
    
        idle_time_end = FILETIME()
        kernel32.GetSystemTimes(None, None, ctypes.byref(idle_time_end))
    
        def filetime_to_ms(ft):
            return (ft.dwHighDateTime << 32) | ft.dwLowDateTime
    
        idle_time_start_ms = filetime_to_ms(idle_time_start)
        idle_time_end_ms = filetime_to_ms(idle_time_end)
    
        time_difference_ms = (idle_time_end_ms - idle_time_start_ms) // 10000  # Конвертация в секунды
    
        return time_difference_ms

  Функция _get_context_switches_per_second_ делает измерение колчиества переходов контекство в секунду и возвращает результат.

      def start(params):
        result = subprocess.run(["./loader"] + params, capture_output=True, text=True)
        cntMemory, cntCPU, time_t, user_t, sys_t, wait_t = list(map(float, result.stdout.split()))
    
        return cntMemory, cntCPU, time_t, user_t, sys_t, wait_t
 Функция _start_ отвечает за запуск программы на C++, которая ранее была скомпилирована в файл loader.exe (или просто loader) и находится в той же директории, что и программа на Python. _subprocess.run()_ отвечает за запуск программы на C++, куда передает путь до скомпилированного файла, параметры, принимаемые функцией main в C++, а также некоторые установки. Например, capture_output позволяет установить флаг того, ожидаем ли мы вывод программы. В нашем случае — да. Данные по запуску идут в переменную result. _result.stdout.split()_ получает данные из программы на плюсах в строчном виде и разбивает их на отдельные значения командой split(). list и map используются, чтобы сразу при разделении сделать значения вещественным типом float. И, собственно, возвращает полученные данные в типе float.
 
    def run():
    try:
        fileName = input("Введите имя файла для сохранения>> ")

        f = open(fileName, 'w')
        f.write('Memory;CPU;TIME;USER%;SYS%;WAIT%;CountContext\n')
        parameters = [ [0, 1], [0, 2], [0, 3], [0,4], [0, 5], [0, 6], [0, 7], [0, 9], [0, 10], [0, 12], [1, 0], [2, 0], [3, 0], [4, 0], [5, 0], [6, 0], [8, 0], [10, 0], [12, 0], [1, 1], [1, 2], [2, 1], [3, 3], [5, 7], [7, 5], [6, 6], [2, 10], [10, 2]  ];

        for params in parameters:
            print(params)
            params[0] = str(params[0])
            params[1] = str(params[1])
            # Запускаем C++ программу, предполагаем, что она скомпилирована и находится в текущей директории
            time, user, sys, wait = 0, 0, 0, 0
            countIter = 20
            cntContext = 0

            for i in range(countIter):
                executor = ThreadPoolExecutor(max_workers=2)

                counter_of_context = executor.submit(get_context_switches_per_second)
                s = executor.submit(start, params)
                

                
                cntMemory, cntCPU, time_t, user_t, sys_t, wait_t =s.result()
                cntContext += counter_of_context.result()

                
                time += time_t
                user += user_t
                sys += sys_t
                wait += wait_t
            f.write(f'{int(cntMemory)};{int(cntCPU)};{time/countIter};{user/countIter};{sys/countIter};{wait/countIter};{cntContext/countIter}\n')
            print('Удачно')

    except Exception as e:
        print(f"Ошибка при запуске программы: {e}")

Функция _run_ — ключевая функция. В переменную filename принимает ввод с клавиатуры, который является именем файла. В переменной f идет открытие файла с ключом 'w' (который говорит, что мы в этот файл записываем). Если файла такого не было, то программа сама его создаст и откроет. _Parameters_ — это набор пар: (countThreadsMemory, countThreadsCPU). Это для программы CPU. В программе идет перебор этих пар циклом for, где идет конвертация значений в строки. Устанавливаем значения time, user, sys, wait на нули. Эти переменные будут накапливать сумму всех измерений на каждую пару параметров. countIter — количество измерений для одной пары параметров. Для того чтобы делать параллельное измерение количества переключений контекстов, создаются два потока. Один для запуска самой программы на плюсах, а другой — для измерений. За это отвечает:

     executor = ThreadPoolExecutor(max_workers=2)

                counter_of_context = executor.submit(get_context_switches_per_second)
                s = executor.submit(start, params)
                

                
                cntMemory, cntCPU, time_t, user_t, sys_t, wait_t =s.result()
                cntContext += counter_of_context.result()
С помощью `.result` получаем возвращаемые значения функциями, запущенными в потоках. 

Затем с помощью `_f.write()` идет запись среднеарифметических значений всех полученных результатов измерений. Идет вывод «удачно», если все прошло хорошо. Если вдруг программа получает ошибку, то идет ее обработка в `except`.

# Результат анализа без агрессивной оптимизации

В ходе анализа были сделаны измерения времени для ситуаций, когда запущен один нагрузчик на CPU и больше, а нагрузчиков на CPU = 0, когда наоборот, и когда одновременно было запущено несколько нагрузчиков на CPU и память. Суммарное количество нагрузчиков не привыщает 12, поскольку в моем компьютере 12 логических процессов. Измерение USER%, SYS% и WAIT% считаются данными из инструментов профилирования. 

Первый график показывает отношение времени от количества запущенных нагрузчиков. 


![График зависимости времени от количества CPU](https://github.com/user-attachments/assets/a1bcff0e-174a-4970-9324-e195d8a7e4d5)

На графике видно, что чем больше запущенных нагрузчиков на CPU, тем больше времени выполняется. Пик достигается при 9-ти запущенных нагрузчиков.

Посомтрим зависимость времени от количества нагрузчиков на дисковую память

![График зависимости времени от количества Memory](https://github.com/user-attachments/assets/87e5fd6d-91a9-4ea9-ad2d-d60450214cb6)

На графике видны скачки времени. Однако можно заметить, что когда нет нагрузчиков CPU, время мало. Пик времени был при 4-х нагрузчиках на память.

Посмотрим на график времени, когда запущено одновременно несколько нагрузчиков двух видов. Где пара a-b — это пара countThreadsMemory-countThreadsCPU.

![График зависимости времени от CPU и Memory](https://github.com/user-attachments/assets/44a2deca-48fe-42ea-9aee-b00c0a103bc9)

Если посмотреть на график, то можно заметить, что теперь при время работы общее больше тогда, когда больше запущенно нагрузчиков на CPU. Это говорит о том, как нагрузчики на CPU влияют на скорость чтения: чем больше нагрузки на CPU, тем медленнее чтение. То есть нагрузка CPU влияет на чтение с диска. 

Далее просто приложу график зависимости USER%, SYS%, WAIT% и количества переключений контекста от количества запущенных нагрузчиков. 

![График зависимости метрик от нагрузчиков](https://github.com/user-attachments/assets/a0181c33-0416-4ac1-a162-783582c18819)



# Анализ метрик с агрессивной оптимизацией

Прелагаю разом посмотреть на все графики.

![График метрик при агрессивной оптимизации](https://github.com/user-attachments/assets/2aad1b3f-98b9-49e1-89e2-3e7ce8db2f48)



Можно заметить, что численные показания времени на каждом графике (первые три) гораздо меньше, чем соответствующие показания ранее. Даже в случае наличия нагрузки на дисковую память и отсутствия нагрузки на CPU время равняется почти всегда 0, а максимум = 0.008.

# Вывод
В ходе выполнения лабораторной работы я разработал программу-оболочку для Shell и несколько программ, которые создают нагрузку, на языке C++. Это позволило мне освежить в памяти основы программирования на C++, а также углубить знания о своей операционной системе.

Работа была особенно увлекательной, поскольку я выполнял её на новом ноутбуке Lunnen — творении молодого российского бренда от Яндекса. В процессе исследования я изучал, как нагрузка на центральный процессор (CPU) и оперативную память влияет на время работы системы.

Результаты показали, что нагрузка на память практически не сказывается на работе системы и программ при небольшой нагрузке на CPU. Однако, как только запускается хотя бы несколько нагрузочных программ на CPU, время работы значительно увеличивается. Это говорит о том, что нагрузка на CPU влияет на процесс чтения с диска.

На графике видно, что использование процессора для пользовательских программ резко возрастает, когда нагрузка на память высока, а на CPU — низкая. В целом, USER% увеличивается с увеличением нагрузки на память.

Но показатели метрики SYS% примерно равны, когда нагрузка на CPU высокая, а на память — низкая.

А вот метрика WAIT всегда высока и примерно одинакова.
