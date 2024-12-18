
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

 func fill(limit) {
    var arr = [];
    var k = 10;
    var resIndex = 0;
    while (resIndex < limit) {
        arr[resIndex] = k;
        resIndex = resIndex + 1;
        k = k - 2;
    }
    return arr;
}

var array = fill(5);

var size = 5;
var sortedArray = bubbleSort(array, size);
array;

)");
auto z=AS_ARRAY(result);

    int x=10;
    result.type;
}