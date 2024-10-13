cmake_minimum_required(VERSION 3.23)
project(virtual_machine)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/virtual_machine)

# Enable testing
enable_testing()

# Include FetchContent module for downloading dependencies
include(FetchContent)

# Declare and fetch Google Test
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)

# Prevent Google Test from overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Make Google Test available
FetchContent_MakeAvailable(googletest)

# Create a library from your virtual machine source files
# It's better to separate headers and sources, but for simplicity, we'll list all files
add_library(vm_lib STATIC
        virtual_machine/EvaluationValue.h
        virtual_machine/vm.h
        virtual_machine/Logger.h
        virtual_machine/OpCode.h
        virtual_machine/parser.h
        virtual_machine/virtual_machine.cpp
        virtual_machine/bytecodeGenerator.h
)

# Optionally, specify include directories for the library (if not using include_directories globally)
# target_include_directories(vm_lib PUBLIC ${PROJECT_SOURCE_DIR}/virtual_machine)

# Create the main executable and link it against vm_lib
add_executable(virtual_machine
        virtual_machine/virtual_machine.cpp # Typically, only the main.cpp is added here
)

# Link the main executable with the library
target_link_libraries(virtual_machine PRIVATE vm_lib)

# Collect all test source files
# It's better to specify test files explicitly for clarity and to avoid unexpected issues
set(TEST_SOURCES
        tests/tests.cpp
        virtual_machine/disassembler/Disassembler.h
        # Add other test source files here
)

# Create the test executable and link it against Google Test and vm_lib
add_executable(MyTests ${TEST_SOURCES})

# Link the test executable with Google Test and vm_lib
target_link_libraries(MyTests
        GTest::gtest
        GTest::gtest_main
        vm_lib
)

# Discover tests using Google Test
include(GoogleTest)
gtest_discover_tests(MyTests)
