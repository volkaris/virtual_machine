// tests/tests.cpp
#include <gtest/gtest.h>
#include "vm.h"

// Test fixture for Vm tests
class VmTest : public ::testing::Test {
protected:
    void SetUp() override {
        _vm = new vm();
    }

    void TearDown() override {
        delete _vm;
    }

    vm* _vm;
};

// Test 1: If condition is false (5 > 10)
TEST_F(VmTest, IfConditionFalse) {
    auto result = _vm->exec(R"(
        (if (5 > 10) 1 2);
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 2) << "Expected result to be 2.";
}

// Test 2: If condition is true (10 > 5)
TEST_F(VmTest, IfConditionTrue) {
    auto result = _vm->exec(R"(
        (if (10 > 5) 1 2);
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 1) << "Expected result to be 1.";
}

// Test 3: Nested if expressions
TEST_F(VmTest, NestedIfExpressions) {
    auto result = _vm->exec(R"(
        (if (5 > 3)
            (if (2 > 1) 100 200)
            (if (3 > 2) 300 400)
        );
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 100) << "Expected result to be 100.";
}

// Test 4: Multiple if expressions in sequence
TEST_F(VmTest, MultipleIfExpressions) {
    auto result1 = _vm->exec(R"(
        (if (1 == 1) 10 20);
    )");
    auto result2 = _vm->exec(R"(
        (if (2 == 3) 30 40);
    )");

    EXPECT_TRUE(IS_NUMBER(result1)) << "Expected result1 to be a number.";
    EXPECT_EQ(AS_NUMBER(result1), 10) << "Expected result1 to be 10.";

    EXPECT_TRUE(IS_NUMBER(result2)) << "Expected result2 to be a number.";
    EXPECT_EQ(AS_NUMBER(result2), 40) << "Expected result2 to be 40.";
}

// Test 5: Complex expressions in condition and branches
TEST_F(VmTest, ComplexExpressions) {
    auto result = _vm->exec(R"(
        (if ((5 + 5) == (2 * 5))
            (10 * 10)
            (20 * 20)
        );
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 100) << "Expected result to be 100.";
}

// Test 6: If with boolean literals
TEST_F(VmTest, IfWithBooleanLiterals) {
    auto result = _vm->exec(R"(
        (if true 1 2);
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 1) << "Expected result to be 1.";
}

// Test 7: If with string comparison
TEST_F(VmTest, IfWithStringComparison) {
    auto result = _vm->exec(R"(
        (if ("hello" == "hello") "yes" "no");
    )");

    EXPECT_TRUE(IS_STRING(result)) << "Expected result to be a string.";
    EXPECT_EQ(AS_CPP_STRING(result), "yes") << "Expected result to be 'yes'.";
}

// Test 8: Chain of if expressions
TEST_F(VmTest, ChainOfIfExpressions) {
    auto result = _vm->exec(R"(
        (if false
            1
            (if false
                2
                (if true
                    3
                    4
                )
            )
        );
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 3) << "Expected result to be 3.";
}

// Test 9: Division by Zero (Expected to throw)
TEST_F(VmTest, DivisionByZero) {
    EXPECT_THROW({
        _vm->exec("(5 / 0);");
    }, std::exception); // Replace with specific exception if available
}

// Test 10: Simple Addition
TEST_F(VmTest, SimpleAddition) {
    auto result = _vm->exec("(5 + 10);");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 15) << "Expected result to be 15.";
}

// Test 11: String Concatenation
TEST_F(VmTest, StringConcatenation) {
    auto result = _vm->exec(R"(
        ("Hello" + " World");
    )");

    EXPECT_TRUE(IS_STRING(result)) << "Expected result to be a string.";
    EXPECT_EQ(AS_CPP_STRING(result), "Hello World") << "Expected result to be 'Hello World'.";
}

// Test 12: Invalid Operation (String + Number)
TEST_F(VmTest, InvalidOperationStringNumber) {
    EXPECT_THROW({
        _vm->exec(R"(
            ("Hello" + 5);
        )");
    }, std::exception); // Replace with specific exception if available
}

// Test 13: Greater Than Comparison
TEST_F(VmTest, GreaterThanComparison) {
    auto result = _vm->exec("(5 > 3);");

    EXPECT_TRUE(IS_BOOLEAN(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOLEAN(result)) << "Expected 5 > 3 to be true.";
}

// Test 14: Less Than Comparison
TEST_F(VmTest, LessThanComparison) {
    auto result = _vm->exec("(2 < 4);");

    EXPECT_TRUE(IS_BOOLEAN(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOLEAN(result)) << "Expected 2 < 4 to be true.";
}

// Test 15: Equality Comparison
TEST_F(VmTest, EqualityComparison) {
    auto result = _vm->exec("(5 == 5);");

    EXPECT_TRUE(IS_BOOLEAN(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOLEAN(result)) << "Expected 5 == 5 to be true.";
}

// Test 16: Greater Than or Equal Comparison
TEST_F(VmTest, GreaterThanOrEqualComparison) {
    auto result = _vm->exec("(5 >= 5);");

    EXPECT_TRUE(IS_BOOLEAN(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOLEAN(result)) << "Expected 5 >= 5 to be true.";
}

// Test 17: Less Than or Equal Comparison
TEST_F(VmTest, LessThanOrEqualComparison) {
    auto result = _vm->exec("(3 <= 5);");

    EXPECT_TRUE(IS_BOOLEAN(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOLEAN(result)) << "Expected 3 <= 5 to be true.";
}

// Test 18: Not Equal Comparison
TEST_F(VmTest, NotEqualComparison) {
    auto result = _vm->exec("(5 != 3);");

    EXPECT_TRUE(IS_BOOLEAN(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOLEAN(result)) << "Expected 5 != 3 to be true.";
}

// Test 19: Invalid Comparison
TEST_F(VmTest, InvalidComparison) {
    auto result = _vm->exec("(5 < 3);");

    EXPECT_TRUE(IS_BOOLEAN(result)) << "Expected result to be a boolean.";
    EXPECT_FALSE(AS_BOOLEAN(result)) << "Expected 5 < 3 to be false.";
}

// Test 20: Chained Operations
TEST_F(VmTest, ChainedOperations) {
    auto result = _vm->exec("((5 + 10) * (2 + 3));");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 75) << "Expected result to be 75.";
}

// Test 21: Deeply Nested Expressions
TEST_F(VmTest, DeeplyNestedExpressions) {
    auto result = _vm->exec("(((1 + 2) + 3) + 4);");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 10) << "Expected result to be 10.";
}

// Test 22: Deconstruct Program By Assembler
TEST_F(VmTest, DeconstructProgramByAssem) {
    auto result = _vm->exec("10;");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 10) << "Expected result to be 10.";
}

// Test 23: Deconstruct Program Sum By Assembler
TEST_F(VmTest, DeconstructSum) {
    auto result = _vm->exec("(5 + 2);");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 7) << "Expected result to be 7.";
}

// Test 24: Deconstruct Program Difference By Assembler
TEST_F(VmTest, DeconstructDifference) {
    auto result = _vm->exec("(if (5 > 10) 1 2);");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 2) << "Expected result to be 2.";
}

// Test 25: Variable Declaration and Usage
TEST_F(VmTest, VariableDeclarationAndUsage) {
    auto result = _vm->exec(R"(
        var x = 10;
        var y = 20;
        (if (x > y) 1 2);
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 2) << "Expected result to be 2.";
}

// Test 26: Variable Modification and Access
TEST_F(VmTest, VariableModificationAndAccess) {
    auto result = _vm->exec(R"(
        var x = 5;
        x = x + 10;
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 15) << "Expected result to be 15.";
}


//todo implement variable shadowing
/*// Test 27: Variable Shadowing
TEST_F(VmTest, VariableShadowing) {
    auto result = _vm->exec(R"(
        var x = 5;
        var y = x + 5;
        var x = y + 5; // New 'x' shadows the previous 'x'
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 15) << "Expected result to be 15.";
}*/

// Test 28: Undefined Variable Access (Should Throw)
TEST_F(VmTest, UndefinedVariableAccess) {
    EXPECT_THROW({
        _vm->exec(R"(
            var x = 10;
            y = x + 5; // 'y' is not declared
        )");
    }, std::exception);
}

// Test 29: Multiple Assignments
TEST_F(VmTest, MultipleAssignments) {
    auto result = _vm->exec(R"(
        var a = 1;
        var b = 2;
        var c = 3;
        a = b;
        b = c;
        c = a + b;
        c;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 5) << "Expected result to be 5.";
}

// Test 30: Assignment to Global Variable
TEST_F(VmTest, AssignmentToGlobalVariable) {
    // Assuming 'x' is a global variable initialized in 'setGlobalVariables'
    auto result = _vm->exec(R"(
        x = x + 10;
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 20) << "Expected result to be 20 (assuming initial x=10).";
}

// Test 31: Assignment Before Declaration (Should Throw)
TEST_F(VmTest, AssignmentBeforeDeclaration) {
    EXPECT_THROW({
        _vm->exec(R"(
            a = 10; // 'a' is not declared yet
            var a = 5;
        )");
    }, std::exception);
}

// Test 32: Chained Variable Usage
TEST_F(VmTest, ChainedVariableUsage) {
    auto result = _vm->exec(R"(
        var a = 2;
        var b = a * 3;
        var c = b + a;
        c;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 8) << "Expected result to be 8.";
}


// Test 33: Redeclaration of Variable (Should Throw)
TEST_F(VmTest, RedeclarationOfVariable) {
    EXPECT_THROW({
        _vm->exec(R"(
            var x = 10;
            var x = 20; // Redeclaration of 'x' should throw an error
        )");
    }, std::exception);
}


// Test 34: Using Variable Before Declaration (Should Throw)
TEST_F(VmTest, UsingVariableBeforeDeclaration) {
    EXPECT_THROW({
        _vm->exec(R"(
            y = 10; // 'y' is used before declaration
            var y = 5;
        )");
    }, std::exception);
}
