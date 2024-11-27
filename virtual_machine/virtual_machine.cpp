#include <iostream>
#include "vm.h"

using namespace std;


int main() {
    vm vm;


    auto result = vm.exec(R"(
    var x = 0;
    while (x < 5) {
        x = x + 1;
    }
    x;
)");

    int i = 0;
    int total = 0;
    while (i < 3) {
        int j = 0;
        while (j < 3) {
            total = total + (i * 3 + j);
            j = j + 1;
        }
        i = i + 1;
    }
    std::cout << total;
int     x=10;

}
