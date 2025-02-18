import subprocess
import ctypes
import time
import threading
from concurrent.futures import ThreadPoolExecutor



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

def start(params):
    result = subprocess.run(["./loader"] + params, capture_output=True, text=True) #
    cntMemory, cntCPU, time_t, user_t, sys_t, wait_t = list(map(float, result.stdout.split()))

    return cntMemory, cntCPU, time_t, user_t, sys_t, wait_t

def run_cpp_program():
    try:
        fileName = input("Введите имя файла для сохранения>> ")

        f = open(fileName, 'w')
        f.write('Memory;CPU;TIME;USER%;SYS%;WAIT%;CountContext\n')
        parameters = [ [0, 1], [0, 2], [0, 3], [0,4], [0, 5], [0, 6], [0, 7], [0, 9], [0, 10], [0, 12], [1, 0], [2, 0], [3, 0], [4, 0], [5, 0], [6, 0], [8, 0], [10, 0], [12, 0], [1, 1], [1, 2], [2, 1], [3, 3], [5, 7], [7, 5], [6, 6], [2, 10], [10, 2]  ];

        for params in parameters:
            print(params)
            params[0] = str(params[0])
            params[1] = str(params[1])
            
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

if __name__ == "__main__":
    run_cpp_program()
