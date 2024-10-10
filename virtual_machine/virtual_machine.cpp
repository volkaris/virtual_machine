#include <iostream>
#include "Vm.h"

using namespace std;


int main () {
    Vm vm;

    {
        auto result = vm.exec(R"(
            (if (5 > 3)
                (if (2 > 1) 100 200)
                (if (3 > 2) 300 400)
            )
        )");
        if (IS_NUMBER(result)) {
            cout << "Result: " << AS_NUMBER(result) << endl;  // Expected output: Result: 2
        } else {
            cout << "Unexpected result type." << endl;
        }
    }


    {
        auto result = vm.exec("( false )");
        if (IS_BOOLEAN(result)) {
            std::cout << "Result: " << AS_BOOLEAN(result) << std::endl;;
        }
        std::cout << "---------------------------" << std::endl;
    }

    // Test greater than (>)
    {
        auto result = vm.exec("(5 > 3)");
        if (IS_BOOLEAN(result)) {
            std::cout << "5 > 3: " << (AS_BOOLEAN(result) ? "true" : "false") << std::endl;
        }
        else {
            std::cout << "5 > 3: Error - Result is not a boolean." << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    // Test less than (<)
    {
        auto result = vm.exec("(2 < 4)");
        if (IS_BOOLEAN(result)) {
            std::cout << "2 < 4: " << (AS_BOOLEAN(result) ? "true" : "false") << std::endl;
        }
        else {
            std::cout << "2 < 4: Error - Result is not a boolean." << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    // Test equality (==)
    {
        auto result = vm.exec("(5 == 5)");
        if (IS_BOOLEAN(result)) {
            std::cout << "5 == 5: " << (AS_BOOLEAN(result) ? "true" : "false") << std::endl;
        }
        else {
            std::cout << "5 == 5: Error - Result is not a boolean." << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    // Test greater than or equal (>=)
    {
        auto result = vm.exec("(5 >= 5)");
        if (IS_BOOLEAN(result)) {
            std::cout << "5 >= 5: " << (AS_BOOLEAN(result) ? "true" : "false") << std::endl;
        }
        else {
            std::cout << "5 >= 5: Error - Result is not a boolean." << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    // Test less than or equal (<=)
    {
        auto result = vm.exec("(3 <= 5)");
        if (IS_BOOLEAN(result)) {
            std::cout << "3 <= 5: " << (AS_BOOLEAN(result) ? "true" : "false") << std::endl;
        }
        else {
            std::cout << "3 <= 5: Error - Result is not a boolean." << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    // Test not equal (!=)
    {
        auto result = vm.exec("(5 != 3)");
        if (IS_BOOLEAN(result)) {
            std::cout << "5 != 3: " << (AS_BOOLEAN(result) ? "true" : "false") << std::endl;
        }
        else {
            std::cout << "5 != 3: Error - Result is not a boolean." << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    // Additional test: False comparison
    {
        auto result = vm.exec("(5 < 3)");
        if (IS_BOOLEAN(result)) {
            std::cout << "5 < 3: " << (AS_BOOLEAN(result) ? "true" : "false") << std::endl;
        }
        else {
            std::cout << "5 < 3: Error - Result is not a boolean." << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }


    // Test 1: Simple Addition
    {
        std::cout << "Test 1: Simple Addition (5 + 10)" << std::endl;
        auto result = vm.exec("(5 + 10)");
        if (IS_NUMBER(result)) {
            std::cout << "Result: " << AS_NUMBER(result) << " (Expected: 15)" << std::endl;
        }
        else {
            std::cout << "Result Type: Unknown (Expected: Number)" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    // Test 2: Simple Multiplication
    {
        std::cout << "Test 2: Simple Multiplication (5 * 10)" << std::endl;
        auto result = vm.exec("(5 * 10)");
        if (IS_NUMBER(result)) {
            std::cout << "Result: " << AS_NUMBER(result) << " (Expected: 50)" << std::endl;
        }
        else {
            std::cout << "Result Type: Unknown (Expected: Number)" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    // Test 3: Nested Expressions ((5 + 10) * 2)
    {
        std::cout << "Test 3: Nested Expressions ((5 + 10) * 2)" << std::endl;
        auto result = vm.exec("((5 + 10) * 2)");
        if (IS_NUMBER(result)) {
            std::cout << "Result: " << AS_NUMBER(result) << " (Expected: 30)" << std::endl;
        }
        else {
            std::cout << "Result Type: Unknown (Expected: Number)" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    // Test 4: String Concatenation ("Hello" + " World")
    {
        std::cout << "Test 4: String Concatenation (\"Hello\" + \" World\")" << std::endl;
        auto result = vm.exec("(\"Hello\" + \" World\")");
        if (IS_STRING(result)) {
            std::cout << "Result: " << AS_CPP_STRING(result) << " (Expected: Hello World)" << std::endl;
        }
        else {
            std::cout << "Result Type: Unknown (Expected: String)" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    // Test 5: Division (20 / 4)
    {
        std::cout << "Test 5: Division (20 / 4)" << std::endl;
        auto result = vm.exec("(20 / 4)");
        if (IS_NUMBER(result)) {
            std::cout << "Result: " << AS_NUMBER(result) << " (Expected: 5)" << std::endl;
        }
        else {
            std::cout << "Result Type: Unknown (Expected: Number)" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    // Test 6: Complex Nested Expressions ((5 + (3 * 2)) / 2)
    {
        std::cout << "Test 6: Complex Nested Expressions ((5 + (3 * 2)) / 2)" << std::endl;
        auto result = vm.exec("((5 + (3 * 2)) / 2)");
        if (IS_NUMBER(result)) {
            std::cout << "Result: " << AS_NUMBER(result) << " (Expected: 5.5)" << std::endl;
        }
        else {
            std::cout << "Result Type: Unknown (Expected: Number)" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    // Test 7: String and Number Addition (Should Error)
    {
        std::cout << "Test 7: Invalid Operation (\"Hello\" + 5)" << std::endl;
        try {
            auto result = vm.exec("(\"Hello\" + 5)");
            // If no exception, print the result
            if (IS_STRING(result)) {
                std::cout << "Result: " << AS_CPP_STRING(result) << " (Expected: Type Error)" << std::endl;
            }
            else if (IS_NUMBER(result)) {
                std::cout << "Result: " << AS_NUMBER(result) << " (Expected: Type Error)" << std::endl;
            }
            else {
                std::cout << "Result Type: Unknown (Expected: Type Error)" << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << "Caught Exception: " << e.what() << " (Expected: Type Error)" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    /*// Test 8: Unknown Operator (5 % 2)
    {
        std::cout << "Test 8: Unknown Operator (5 % 2)" << std::endl;
        try {
            auto result = vm.exec("(5 % 2)");
            // If no exception, print the result
            if (IS_NUMBER(result)) {
                std::cout << "Result: " << AS_NUMBER(result) << " (Expected: Error)" << std::endl;
            } else if (IS_STRING(result)) {
                std::cout << "Result: " << AS_CPP_STRING(result) << " (Expected: Error)" << std::endl;
            } else {
                std::cout << "Result Type: Unknown (Expected: Error)" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Caught Exception: " << e.what() << " (Expected: Unknown Operator Error)" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }*/

    // Test 9: Division by Zero (5 / 0)
    {
        std::cout << "Test 9: Division by Zero (5 / 0)" << std::endl;
        try {
            auto result = vm.exec("(5 / 0)");
            // If no exception, print the result
            if (IS_NUMBER(result)) {
                std::cout << "Result: " << AS_NUMBER(result) << " (Expected: Error)" << std::endl;
            }
            else {
                std::cout << "Result Type: Unknown (Expected: Error)" << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << "Caught Exception: " << e.what() << " (Expected: Division by Zero Error)" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    /*// Test 10: Invalid Syntax (5 + )
    {
        std::cout << "Test 10: Invalid Syntax (5 + )" << std::endl;
        try {
            auto result = vm.exec("(5 + )");
            // If no exception, print the result
            if (IS_NUMBER(result)) {
                std::cout << "Result: " << AS_NUMBER(result) << " (Expected: Syntax Error)" << std::endl;
            } else {
                std::cout << "Result Type: Unknown (Expected: Syntax Error)" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Caught Exception: " << e.what() << " (Expected: Syntax Error)" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }*/

    // Test 11: Large Numbers (1000000 + 2000000)
    {
        std::cout << "Test 11: Large Numbers (1000000 + 2000000)" << std::endl;
        auto result = vm.exec("(1000000 + 2000000)");
        if (IS_NUMBER(result)) {
            std::cout << "Result: " << AS_NUMBER(result) << " (Expected: 3000000)" << std::endl;
        }
        else {
            std::cout << "Result Type: Unknown (Expected: Number)" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    // Test 12: Multiple Operations (5 + 10 - 3)
    {
        std::cout << "Test 12: Multiple Operations (5 + 10 - 3)" << std::endl;
        auto result = vm.exec("((5 + 10) - 3)");
        if (IS_NUMBER(result)) {
            std::cout << "Result: " << AS_NUMBER(result) << " (Expected: 12)" << std::endl;
        }
        else {
            std::cout << "Result Type: Unknown (Expected: Number)" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    /*// Test 13: Combined String and Number (Should Error)
    {
        std::cout << "Test 13: Combined String and Number (\"Number: \" + 5)" << std::endl;
        try {
            auto result = vm.exec("(\"Number: \" + 5)");
            // If no exception, print the result
            if (IS_STRING(result)) {
                std::cout << "Result: " << AS_CPP_STRING(result) << " (Expected: Type Error)" << std::endl;
            } else if (IS_NUMBER(result)) {
                std::cout << "Result: " << AS_NUMBER(result) << " (Expected: Type Error)" << std::endl;
            } else {
                std::cout << "Result Type: Unknown (Expected: Type Error)" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Caught Exception: " << e.what() << " (Expected: Type Error)" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }*/

    // Test 14: Chained Operations ((5 + 10) * (2 + 3))
    {
        std::cout << "Test 14: Chained Operations ((5 + 10) * (2 + 3))" << std::endl;
        auto result = vm.exec("((5 + 10) * (2 + 3))");
        if (IS_NUMBER(result)) {
            std::cout << "Result: " << AS_NUMBER(result) << " (Expected: 75)" << std::endl;
        }
        else {
            std::cout << "Result Type: Unknown (Expected: Number)" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    // Test 15: Deeply Nested Expressions (((1 + 2) + 3) + 4)
    {
        std::cout << "Test 15: Deeply Nested Expressions (((1 + 2) + 3) + 4)" << std::endl;
        auto result = vm.exec("(((1 + 2) + 3) + 4)");
        if (IS_NUMBER(result)) {
            std::cout << "Result: " << AS_NUMBER(result) << " (Expected: 10)" << std::endl;
        }
        else {
            std::cout << "Result Type: Unknown (Expected: Number)" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    return 0;
}
