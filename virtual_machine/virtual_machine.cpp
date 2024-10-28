#include <iostream>
#include "vm.h"

using namespace std;


int main() {
    vm vm;
    /*EvaluationValue result = vm.exec("var x = 10; x = x + 5;  x ");*/

    auto result = vm.exec(R"(
            {
                var x = 10;
            }
            x; // 'x' should not be accessible here
        )");
    return 0;
}
