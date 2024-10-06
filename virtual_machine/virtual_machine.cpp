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
    }

	// Process the result as needed
	return 0;
}
