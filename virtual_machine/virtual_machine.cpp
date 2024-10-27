#include <iostream>
#include "vm.h"

using namespace std;


int main() {
    vm vm;
    /*EvaluationValue result = vm.exec("var x = 10; x = x + 5;  x ");*/

  auto res2=  vm.exec(R"(
            y = 10; // 'y' is used before declaration
            var y = 5;

            y;
        )");
    return 0;
}
