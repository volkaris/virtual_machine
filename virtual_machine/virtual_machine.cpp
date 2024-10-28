#include <iostream>
#include "vm.h"

using namespace std;


int main() {
    vm vm;
    /*EvaluationValue result = vm.exec("var x = 10; x = x + 5;  x ");*/

    auto result = vm.exec(R"(
           var x=10;
           if (x!=10) {
            var y=100;
            var z=200;
            var yz=y+z;
            yz;
  }
            else {
                y;
}
        )");
    return 0;
}
