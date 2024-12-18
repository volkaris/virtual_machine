#include <iostream>
#include "vm.h"
using namespace std;
int main() {
    vm vm;
    auto result = vm.exec(R"(

func bubbleSort(arr, n) {

var i = 0;
    while (i < n) {
        var j = 0;
        while (j < n - i - 1) {
            if (arr[j] > arr[j + 1]) {
                var temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
            j = j + 1;
        }
        i = i + 1;
    }
    return arr;
}

var array = [5, 3, 8, 6, 2];
var size = 5;
var sortedArray = bubbleSort(array, size);
sortedArray;

)");
auto z=AS_ARRAY(result);

    int x=10;
    result.type;
}