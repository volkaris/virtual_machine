#include <iostream>
#include "vm.h"

using namespace std;


int main() {
    vm vm;

auto res=vm.exec(R"(func factorial(n) {
    return 1;
}

var x = factorial(5);)");
int x=10;
}