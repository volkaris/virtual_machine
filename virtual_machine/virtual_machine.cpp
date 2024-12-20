#include <fstream>
#include <iostream>
#include "vm.h"
using namespace std;

int main() {


    std::string filePath;

    cin >> filePath;

    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Could not open the file: " << filePath << std::endl;
        return 1;
    }


    std::string code((std::istreambuf_iterator(file)),
                     std::istreambuf_iterator<char>());

    file.close();


    vm machine;

    machine.exec(code);
}
