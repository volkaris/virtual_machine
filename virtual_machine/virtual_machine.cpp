#include <iostream>

#include "Logger.h"
#include "vm.h"
#include "EvaluationValue.h"

using namespace std;


int main() {
    vm vm;
    auto result = vm.exec(R"(

      x

    )");

    std::cout << "\n";

    std::cout << "OK" << "\n";

    return 0;
}
