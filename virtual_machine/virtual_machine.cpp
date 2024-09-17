#include <iostream>
#include "EvaVM.h"
using namespace std;
int main()
{
	EvaVm vm;
	cout<<AS_CPP_STRING(vm.exec(R"( 42 )"));
	

	
}

