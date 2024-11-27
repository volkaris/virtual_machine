#include <iostream>
#include "vm.h"

using namespace std;


int main() {
    vm vm;


    int i = 0;
    for (; !(i >= 5); i = i + 1) {
        // No operation inside the loop
    }
    std::cout << i;

//     ForLoopVariableScope  ForLoopVariableScopeCheck
}
