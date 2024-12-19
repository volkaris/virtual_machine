#include <fstream>
#include <iostream>
#include "vm.h"
using namespace std;

int main() {


    /*setlocale(LC_ALL, "Russian");

    std::string filePath;

    cin >> filePath;

    // Путь к файлу с исходным кодом


    // Открываем файл и читаем его содержимое
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Could not open the file: " << filePath << std::endl;
        return 1;
    }

    // Считываем весь файл в строку
    std::string code((std::istreambuf_iterator(file)),
                     std::istreambuf_iterator<char>());

    file.close();

    // Создаём виртуальную машину
    vm machine;
    // Выполняем считанный код
    machine.exec(code);*/


   vm vm;
    auto result = vm.exec(R"(


func sieveOfEratosthenes(n) {
    // Инициализируем массив чисел от 0 до n,
    // предположим, что все числа - простые (true), кроме 0 и 1
    var sieve = [];
    var i = 0;
    while (i <= n) {
        sieve[i] = true;
        i = i + 1;
    }
    sieve[0] = false;
    sieve[1] = false;

    var p = 2;
    while (p * p <= n) {
        if (sieve[p] == true) {
            var j = p * p;
            while (j <= n) {
                sieve[j] = false;
                j = j + p;
            }
        }
        p = p + 1;
    }
    return sieve;
}



var primes = sieveOfEratosthenes(30);

print(primes);

)");

    int x=10;
}
