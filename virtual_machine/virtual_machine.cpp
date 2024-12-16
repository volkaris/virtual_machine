#include <iostream>
#include "vm.h"

using namespace std;


int main() {
    vm vm;

    auto result = vm.exec(R"(
        var number = 5;

func factorial(n) {
    if (n == 0) {
        var result = 1;
    } else {
        var temp = factorial(n - 1);
        var result = n * temp;
    }
}

var fact = factorial(number);
    )");

    int x=10;

//     ForLoopVariableScope  ForLoopVariableScopeCheck
}
