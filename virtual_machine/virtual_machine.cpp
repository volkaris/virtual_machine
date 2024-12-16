#include <iostream>
#include "vm.h"

using namespace std;


int main() {
    vm vm;

    auto res = vm.exec(R"(var number = 5;

func factorial(n) {
    if (n == 0) {
       return 1;
    } else {
       return n * factorial(n - 1);
    }
}

var fact = factorial(number);
fact;)");

    int x=10;
}
