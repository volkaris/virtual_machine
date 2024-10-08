#include <iostream>
#include "Vm.h"

using namespace std;
 

int main() {
	Vm vm;
    auto result = vm.exec(R"(
					((5+10)*2)
                       )");

	if (IS_NUMBER(result)) {
		std::cout << "Result: " << AS_NUMBER(result) << std::endl;
	} else if (IS_STRING(result)) {
		std::cout << "Result: " << AS_CPP_STRING(result) << std::endl;
	} else {
		std::cout << "Unknown result type." << std::endl;
	}

	// Process the result as needed
	return 0;
}
