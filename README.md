## Нагрзучик памяти и CPU

Программа в фале LoaderCPUandMemory.cpp - это код, который предназначен для исследования того, как влияет количество запущенных нагрузчиков либо на чтение данных с дисковой памяти, либо на сортировку сгенерированного массива размером 1000.

Так же идет измерение таких значений: USER%, SYS%, WAIT%

USER% - показывает, сколько времении CPU тратит на выполенение пользовательских процессов
SYS% - сколько времени CPU проводит в режиме ядра
WAIT% - сколько времени CPU простаивает в режиме ожидания ввода/вывода

Программа подразумевает создание файла размером 1Гб. Это сделано специально для того, чтобы нагрузить дисковою память и изменениния в измерениях были заметными. 

Так же есть возможность выбрать количество потоков для запуска нагрузчика на CPU и на память дисковую. Два цикла в main позволяют решить сразу несколько задач: запуск отдельно нагрзучика CPU и на дисковую память, одновременный запуск обоих нагрузчиков. 

Нагрузчик на память дисковую  - randomReadtest. Здесь идет выбор рандомной позиции в файле, откуда идет считывание массива данных размера 1000 в программу. 

Нагрузчик на CPU - startLoaderCPU. Это лишь программа, которая реализует запуск сортировки. Сортировка заключается в том, что несколько нагрузчиков сортируют один массив. Для этого используется сортировка слиянием. Самый удобный метод для сортировки несколькими потоками.



