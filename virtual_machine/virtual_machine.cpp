#include <iostream>
#include "EvaVM.h"

using namespace std;
 

int main() {
	EvaVm vm;
    auto result = vm.exec(R"(
					(+ 5 x)
                       )");

	

	// Process the result as needed
	return 0;
}
