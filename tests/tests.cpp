// tests/tests.cpp
#include <gtest/gtest.h>
#include "vm.h"

// Test fixture for Vm tests
class VmTest : public ::testing::Test {
protected:
    void SetUp() override {
        vm = new vm();
    }

    void TearDown() override {
        delete vm;
    }

    vm* vm;
};

// Test 1: If condition is false (5 > 10)
TEST_F(VmTest, IfConditionFalse) {
    auto result = vm->exec(R"( (if (5 > 10) 1 2 ) )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 2) << "Expected result to be 2.";
}

// Test 2: If condition is true (10 > 5)
TEST_F(VmTest, IfConditionTrue) {
    auto result = vm->exec(R"( (if (10 > 5) 1 2) )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 1) << "Expected result to be 1.";
}

// Test 3: Nested if expressions
TEST_F(VmTest, NestedIfExpressions) {
    auto result = vm->exec(R"(
        (if (5 > 3)
            (if (2 > 1) 100 200)
            (if (3 > 2) 300 400)
        )
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 100) << "Expected result to be 100.";
}

// Test 4: Multiple if expressions in sequence
TEST_F(VmTest, MultipleIfExpressions) {
    auto result1 = vm->exec(R"( (if (1 == 1) 10 20) )");
    auto result2 = vm->exec(R"( (if (2 == 3) 30 40) )");

    EXPECT_TRUE(IS_NUMBER(result1)) << "Expected result1 to be a number.";
    EXPECT_EQ(AS_NUMBER(result1), 10) << "Expected result1 to be 10.";

    EXPECT_TRUE(IS_NUMBER(result2)) << "Expected result2 to be a number.";
    EXPECT_EQ(AS_NUMBER(result2), 40) << "Expected result2 to be 40.";
}

// Test 5: Complex expressions in condition and branches
TEST_F(VmTest, ComplexExpressions) {
    auto result = vm->exec(R"(
        (if ((5 + 5) == (2 * 5))
            (10 * 10)
            (20 * 20)
        )
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 100) << "Expected result to be 100.";
}

// Test 6: If with boolean literals
TEST_F(VmTest, IfWithBooleanLiterals) {
    auto result = vm->exec(R"( (if true 1 2) )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 1) << "Expected result to be 1.";
}

// Test 7: If with string comparison
TEST_F(VmTest, IfWithStringComparison) {
    auto result = vm->exec(R"( (if ("hello" == "hello") "yes" "no") )");

    EXPECT_TRUE(IS_STRING(result)) << "Expected result to be a string.";
    EXPECT_EQ(AS_CPP_STRING(result), "yes") << "Expected result to be 'yes'.";
}

// Test 8: Chain of if expressions
TEST_F(VmTest, ChainOfIfExpressions) {
    auto result = vm->exec(R"(
        (if (false)
            1
            (if (false)
                2
                (if (true)
                    3
                    4
                )
            )
        )
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 3) << "Expected result to be 3.";
}

// Test 9: Division by Zero (Expected to throw)
TEST_F(VmTest, DivisionByZero) {
    EXPECT_THROW({
        vm->exec("(5 / 0)");
    }, std::exception); // Replace with specific exception if available
}

// Test 10: Simple Addition
TEST_F(VmTest, SimpleAddition) {
    auto result = vm->exec("(5 + 10)");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 15) << "Expected result to be 15.";
}

// Test 11: String Concatenation
TEST_F(VmTest, StringConcatenation) {
    auto result = vm->exec("(\"Hello\" + \" World\")");

    EXPECT_TRUE(IS_STRING(result)) << "Expected result to be a string.";
    EXPECT_EQ(AS_CPP_STRING(result), "Hello World") << "Expected result to be 'Hello World'.";
}

// Test 12: Invalid Operation (String + Number)
TEST_F(VmTest, InvalidOperationStringNumber) {
    EXPECT_THROW({
        vm->exec("(\"Hello\" + 5)");
    }, std::exception); // Replace with specific exception if available
}

// Test 13: Greater Than Comparison
TEST_F(VmTest, GreaterThanComparison) {
    auto result = vm->exec("(5 > 3)");

    EXPECT_TRUE(IS_BOOLEAN(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOLEAN(result)) << "Expected 5 > 3 to be true.";
}

// Test 14: Less Than Comparison
TEST_F(VmTest, LessThanComparison) {
    auto result = vm->exec("(2 < 4)");

    EXPECT_TRUE(IS_BOOLEAN(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOLEAN(result)) << "Expected 2 < 4 to be true.";
}

// Test 15: Equality Comparison
TEST_F(VmTest, EqualityComparison) {
    auto result = vm->exec("(5 == 5)");

    EXPECT_TRUE(IS_BOOLEAN(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOLEAN(result)) << "Expected 5 == 5 to be true.";
}

// Test 16: Greater Than or Equal Comparison
TEST_F(VmTest, GreaterThanOrEqualComparison) {
    auto result = vm->exec("(5 >= 5)");

    EXPECT_TRUE(IS_BOOLEAN(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOLEAN(result)) << "Expected 5 >= 5 to be true.";
}

// Test 17: Less Than or Equal Comparison
TEST_F(VmTest, LessThanOrEqualComparison) {
    auto result = vm->exec("(3 <= 5)");

    EXPECT_TRUE(IS_BOOLEAN(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOLEAN(result)) << "Expected 3 <= 5 to be true.";
}

// Test 18: Not Equal Comparison
TEST_F(VmTest, NotEqualComparison) {
    auto result = vm->exec("(5 != 3)");

    EXPECT_TRUE(IS_BOOLEAN(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOLEAN(result)) << "Expected 5 != 3 to be true.";
}

// Test 19: Invalid Comparison
TEST_F(VmTest, InvalidComparison) {
    auto result = vm->exec("(5 < 3)");

    EXPECT_TRUE(IS_BOOLEAN(result)) << "Expected result to be a boolean.";
    EXPECT_FALSE(AS_BOOLEAN(result)) << "Expected 5 < 3 to be false.";
}

// Test 20: Chained Operations
TEST_F(VmTest, ChainedOperations) {
    auto result = vm->exec("((5 + 10) * (2 + 3))");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 75) << "Expected result to be 75.";
}

// Test 21: Deeply Nested Expressions
TEST_F(VmTest, DeeplyNestedExpressions) {
    auto result = vm->exec("(((1 + 2) + 3) + 4)");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 10) << "Expected result to be 10.";
}
