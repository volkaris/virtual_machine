#include <iostream>
#include "vm.h"
using namespace std;
int main() {
    vm vm;
    auto result = vm.exec(R"(

func sieveOfEratosthenes(limit) {
    var primes = [];
    var i = 0;
    while (i <= limit) {
        primes[i] = 1;
        i = i + 1;
    }

    primes[0] = 0;
    primes[1] = 0;

    var p = 2;
    while (p * p <= limit) {
        if (primes[p] == 1) {
            var multiple = p * p;
            while (multiple <= limit) {
                primes[multiple] = 0;
                multiple = multiple + p;
            }
        }
        p = p + 1;
    }

    var result = [];
    var k = 0;
    var resIndex = 0; // Initialize the index variable for result array
    while (k <= limit) {
        if (primes[k] == 1) {
            result[resIndex] = k; // Use resIndex instead of result.length
            resIndex = resIndex + 1; // Increment the index
        }
        k = k + 1;
    }
    return result;
}

var primes = sieveOfEratosthenes(30);
primes;



)");
    int x=10;
    result.type;
}