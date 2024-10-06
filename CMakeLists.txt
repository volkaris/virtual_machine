cmake_minimum_required(VERSION 3.23)
project(virtual_machine)

set(CMAKE_CXX_STANDARD 17)

include_directories(virtual_machine)

add_executable(virtual_machine
        virtual_machine/EvaluationValue.h
        virtual_machine/Vm.h
        virtual_machine/Logger.h
        virtual_machine/OpCode.h
        virtual_machine/parser.h
        virtual_machine/virtual_machine.cpp virtual_machine/compiler.h)
