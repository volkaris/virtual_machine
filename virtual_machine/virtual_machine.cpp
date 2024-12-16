#include <iostream>
#include "vm.h"

using namespace std;


int main() {
    vm vm;

    auto res = vm.exec(R"(


var fact = square(2);


)");

    int x=10;

}

