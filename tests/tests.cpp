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

    vm *_vm;
};

// Test 1: If condition is false (5 > 10)
TEST_F(VmTest, IfConditionFalse) {
    auto result = _vm->exec(R"(
        if (5 > 10) {1;} else {2;}
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 2) << "Expected result to be 2.";
}

// Test 2: If condition is true (10 > 5)
TEST_F(VmTest, IfConditionTrue) {
    auto result = _vm->exec(R"(
        if (10 > 5) {1;} else {2;}
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 1) << "Expected result to be 1.";
}

// Test 3: Nested if expressions
TEST_F(VmTest, NestedIfExpressions) {
    auto result = _vm->exec(R"(
        if (5 > 3) {
            if (2 > 1) {100;} else {200;} }
else {
            if (3 > 2) {300;} else {400;}
        }
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 100) << "Expected result to be 100.";
}

// Test 4: Multiple if expressions in sequence
TEST_F(VmTest, MultipleIfExpressions) {
    auto result1 = _vm->exec(R"(
        if (1 == 1) {10;} else { 20;}
    )");
    auto result2 = _vm->exec(R"(
        if(2 == 3) {30;} else{40;}
    )");

    EXPECT_TRUE(IS_NUMBER(result1)) << "Expected result1 to be a number.";
    EXPECT_EQ(AS_NUMBER(result1), 10) << "Expected result1 to be 10.";

    EXPECT_TRUE(IS_NUMBER(result2)) << "Expected result2 to be a number.";
    EXPECT_EQ(AS_NUMBER(result2), 40) << "Expected result2 to be 40.";
}

// Test 5: Complex expressions in condition and branches
TEST_F(VmTest, ComplexExpressions) {
    auto result = _vm->exec(R"(
        if ((5 + 5) == (2 * 5)) {
            10 * 10;}
else {
            20 * 20;
      }
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 100) << "Expected result to be 100.";
}

// Test 6: If with boolean literals
TEST_F(VmTest, IfWithBooleanLiterals) {
    auto result = _vm->exec(R"(
        if (true) {1;}  else {2;}
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 1) << "Expected result to be 1.";
}

// Test 7: If with string comparison
TEST_F(VmTest, IfWithStringComparison) {
    auto result = _vm->exec(R"(
        if ("hello" == "hello") {"yes";} else{"no";}
    )");

    EXPECT_TRUE(IS_STRING(result)) << "Expected result to be a string.";
    EXPECT_EQ(AS_CPP_STRING(result), "yes") << "Expected result to be 'yes'.";
}

// Test 8: Chain of if expressions
TEST_F(VmTest, ChainOfIfExpressions) {
    auto result = _vm->exec(R"(
        if (false)
            {1;}
        else { if (false)
                {
                    2; }
                if (true)
                    {3;}
                    else{ 4;}
                }
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

    EXPECT_TRUE(IS_BOOL(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOL(result)) << "Expected 5 > 3 to be true.";
}

// Test 14: Less Than Comparison
TEST_F(VmTest, LessThanComparison) {
    auto result = _vm->exec("(2 < 4);");

    EXPECT_TRUE(IS_BOOL(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOL(result)) << "Expected 2 < 4 to be true.";
}

// Test 15: Equality Comparison
TEST_F(VmTest, EqualityComparison) {
    auto result = _vm->exec("(5 == 5);");

    EXPECT_TRUE(IS_BOOL(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOL(result)) << "Expected 5 == 5 to be true.";
}

// Test 16: Greater Than or Equal Comparison
TEST_F(VmTest, GreaterThanOrEqualComparison) {
    auto result = _vm->exec("(5 >= 5);");

    EXPECT_TRUE(IS_BOOL(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOL(result)) << "Expected 5 >= 5 to be true.";
}

// Test 17: Less Than or Equal Comparison
TEST_F(VmTest, LessThanOrEqualComparison) {
    auto result = _vm->exec("(3 <= 5);");

    EXPECT_TRUE(IS_BOOL(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOL(result)) << "Expected 3 <= 5 to be true.";
}

// Test 18: Not Equal Comparison
TEST_F(VmTest, NotEqualComparison) {
    auto result = _vm->exec("(5 != 3);");

    EXPECT_TRUE(IS_BOOL(result)) << "Expected result to be a boolean.";
    EXPECT_TRUE(AS_BOOL(result)) << "Expected 5 != 3 to be true.";
}

// Test 19: Invalid Comparison
TEST_F(VmTest, InvalidComparison) {
    auto result = _vm->exec("(5 < 3);");

    EXPECT_TRUE(IS_BOOL(result)) << "Expected result to be a boolean.";
    EXPECT_FALSE(AS_BOOL(result)) << "Expected 5 < 3 to be false.";
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
    auto result = _vm->exec("if (5 > 10) {1;} else{2;}");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 2) << "Expected result to be 2.";
}

// Test 25: Variable Declaration and Usage
TEST_F(VmTest, VariableDeclarationAndUsage) {
    auto result = _vm->exec(R"(
        var x = 10;
        var y = 20;
        if (x > y) {1;} {2;}
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

/*// Test 30: Assignment to Global Variable
TEST_F(VmTest, AssignmentToGlobalVariable) {
    // Assuming 'x' is a global variable initialized in 'setGlobalVariables'
    auto result = _vm->exec(R"(
        x = x + 10;
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 20) << "Expected result to be 20 (assuming initial x=10).";
}*/

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

// Test 39: Variables in Inner Scope Do Not Affect Outer Scope
TEST_F(VmTest, InnerScopeVariablesDoNotAffectOuterScope) {
    auto result = _vm->exec(R"(
        var x = 10;
        {
            var x = 20; // Inner scope variable 'x' should not affect outer 'x'
            x = x + 5;  // Inner 'x' becomes 25
        }
        x; // Should still be 10
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 10) << "Expected outer 'x' to remain 10.";
}

// Test 40: Variables in Outer Scope Are Accessible in Inner Scope
TEST_F(VmTest, OuterScopeVariablesAccessibleInInnerScope) {
    auto result = _vm->exec(R"(
        var x = 10;
        var y = 0;
        {
            y = x + 5; // Inner scope should access outer 'x'
        }
        y; // Should be 15
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 15) << "Expected 'y' to be 15.";
}

// Test 41: Variable Shadowing Works Correctly
TEST_F(VmTest, VariableShadowing) {
    auto result = _vm->exec(R"(
        var x = 5;
        {
            var x = 10; // Shadows outer 'x'
            x = x + 5;  // Inner 'x' becomes 15
        }
        x; // Should still be 5
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 5) << "Expected outer 'x' to remain 5.";
}

// Test 42: Variables Declared in a Scope Are Not Accessible Outside
TEST_F(VmTest, VariablesNotAccessibleOutsideScope) {
    EXPECT_THROW({
                 _vm->exec(R"(
            {
                var x = 10;
            }
            x; // 'x' should not be accessible here
        )");
                 }, std::exception);
}

// Test 43: Assignment to Variable in Outer Scope from Inner Scope
TEST_F(VmTest, AssignmentToOuterScopeVariableFromInnerScope) {
    auto result = _vm->exec(R"(
        var x = 10;
        {
            x = x + 5; // Modifies outer 'x'
        }
        x; // Should be 15
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 15) << "Expected 'x' to be 15.";
}

// Test 44: Nested Scopes with Variable Declarations
TEST_F(VmTest, NestedScopesWithVariableDeclarations) {
    auto result = _vm->exec(R"(
        var x = 1;
        {
            var y = 2;
            {
                var z = x + y; // z = 1 + 2 = 3
            }
            // z is not accessible here
            y = y + x; // y = 2 + 1 = 3
        }
        x; // Should be 1
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 1) << "Expected 'x' to be 1.";
}

// Test 45: Variable Shadowing in Multiple Nested Scopes
TEST_F(VmTest, VariableShadowingInNestedScopes) {
    auto result = _vm->exec(R"(
        var x = 1;
        {
            var x = 2; // Shadows outer 'x'
            {
                var x = 3; // Shadows previous 'x'
                x; // Should be 3
            }
            x; // Should be 2
        }
        x; // Should be 1
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 1) << "Expected outer 'x' to be 1.";
}

// Test 46: Variable Lifetime Ends with Scope
TEST_F(VmTest, VariableLifetimeEndsWithScope) {
    EXPECT_THROW({
                 auto result = _vm->exec(R"(
            {
                var x = 10;
            }
            x = 20; // Should throw an error, 'x' is out of scope
        )");
                 }, std::exception);
}

// Test 47: Complex Scope Interactions
TEST_F(VmTest, ComplexScopeInteractions) {
    auto result = _vm->exec(R"(
        var x = 1;
        var y = 2;
        {
            var x = y + 1; // x = 3
            y = x + 1;     // y = 4
            {
                var y = x + 1; // y = 4
                x = y + 1;     // x = 5
            }
            x = x + y; // x = 5 + 4 = 9
        }
        x + y; // x = 1, y = 4, so result should be 1 + 4 = 5
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 5) << "Expected result to be 5.";
}

// Test 48: Assignment to Shadowed Variable
TEST_F(VmTest, AssignmentToShadowedVariable) {
    auto result = _vm->exec(R"(
        var x = 10;
        {
            var x = 20; // Shadows outer 'x'
            {
                x = x + 5; // Modifies innermost 'x', x = 25
            }
            x = x + 5; // Modifies inner 'x', x = 30
        }
        x; // Should be 10 (outer 'x' remains unchanged)
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 10) << "Expected outer 'x' to remain 10.";
}


// Test 50: Re-declaring Variable in Same Scope (Should Throw)
TEST_F(VmTest, RedeclaringVariableInSameScope) {
    EXPECT_THROW({
                 _vm->exec(R"(
            var x = 10;
            var x = 20; // Redeclaration in the same scope should throw an error
        )");
                 }, std::exception);
}

// Test 51: Simple If Statement (Condition True)
TEST_F(VmTest, SimpleIfStatementTrue) {
    auto result = _vm->exec(R"(
        var x = 5;
        if (x == 5) {
            x = x + 1;
        }
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 6) << "Expected x to be 6.";
}

TEST_F(VmTest, SimpleIfStatementFalse) {
    auto result = _vm->exec(R"(
        var x = 5;
        if (x != 5) {
            x = x + 1;
        }
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 5) << "Expected x to remain 5.";
}

TEST_F(VmTest, IfElseStatementTrue) {
    auto result = _vm->exec(R"(
        var x = 5;
        if (x == 5) {
            x = x + 1;
        } else {
            x = x - 1;
        }
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 6) << "Expected x to be 6.";
}

TEST_F(VmTest, IfElseStatementFalse) {
    auto result = _vm->exec(R"(
        var x = 5;
        if (x != 5) {
            x = x + 1;
        } else {
            x = x - 1;
        }
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 4) << "Expected x to be 4.";
}

TEST_F(VmTest, NestedIfStatements) {
    auto result = _vm->exec(R"(
        var x = 5;
        var y = 10;
        if (x < y) {
            if (y == 10) {
                x = x + y;
            }
        }
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 15) << "Expected x to be 15.";
}

TEST_F(VmTest, IfElseIfElseChain) {
    auto result = _vm->exec(R"(
        var x = 0;
        if (x > 0) {
            x = 1;
        } else if (x == 0) {
            x = 2;
        } else {
            x = 3;
        }
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 2) << "Expected x to be 2.";
}

TEST_F(VmTest, IfWithoutBraces) {
    auto result = _vm->exec(R"(
        var x = 5;
        if (x == 5)
            x = x + 1;
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 6) << "Expected x to be 6.";
}


TEST_F(VmTest, IfElseWithoutBraces) {
    auto result = _vm->exec(R"(
        var x = 5;
        if (x != 5)
            x = x + 1;
        else
            x = x - 1;
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 4) << "Expected x to be 4.";
}

//IMPORTANT TEST!!! Dangling Else Correct Association
TEST_F(VmTest, DanglingElseCorrectAssociation) {
    auto result = _vm->exec(R"(
        var x = 5;
        if (x > 0)
            if (x > 10)
                x = x + 1;
            else
                x = x - 1;
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 4) << "Expected x to be 4.";
}

TEST_F(VmTest, IfInsideElseBlock) {
    auto result = _vm->exec(R"(
        var x = 5;
        if (x > 10) {
            x = x + 1;
        } else {
            if (x == 5) {
                x = x * 2;
            }
        }
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 10) << "Expected x to be 10.";
}


TEST_F(VmTest, NestedIfElseStatements) {
    auto result = _vm->exec(R"(
        var x = 5;
        if (x > 0) {
            if (x < 10) {
                x = x + 1;
            } else {
                x = x + 2;
            }
        } else {
            x = x - 1;
        }
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 6) << "Expected x to be 6.";
}


//todo implement logical operator ( or,and,not )
/*TEST_F(VmTest, ComplexConditionInIf) {
    auto result = _vm->exec(R"(
        var x = 5;
        var y = 10;
        if ((x < y) && (y == 10) || (x == 0)) {
            x = y / x;
        }
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Expected result to be a number.";
    EXPECT_EQ(AS_NUMBER(result), 2) << "Expected x to be 2.";
}*/

TEST_F(VmTest, LogicalNotOperator) {
    auto result = _vm->exec(R"(
        var x = true;
        if (x) {
            x = false;
        } else {
            x = true;
        }
        x;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Expected result to be a boolean.";
    EXPECT_EQ(AS_BOOL(result), false) << "Expected x to be true.";
}

// Test 72: Logical AND Operator (Both True)
TEST_F(VmTest, LogicalAndBothTrue) {
    auto result = _vm->exec(R"(
        var a = true;
        var b = true;
        var c = a && b;
        c;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Expected result to be a boolean.";
    EXPECT_EQ(AS_BOOL(result), true) << "Expected c to be true.";
}

// Test 73: Logical AND Operator (One False)
TEST_F(VmTest, LogicalAndOneFalse) {
    auto result = _vm->exec(R"(
        var a = true;
        var b = false;
        var c = a && b;
        c;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Expected result to be a boolean.";
    EXPECT_EQ(AS_BOOL(result), false) << "Expected c to be false.";
}

// Test 74: Logical OR Operator (Both False)
TEST_F(VmTest, LogicalOrBothFalse) {
    auto result = _vm->exec(R"(
        var a = false;
        var b = false;
        var c = a || b;
        c;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Expected result to be a boolean.";
    EXPECT_EQ(AS_BOOL(result), false) << "Expected c to be false.";
}

// Test 75: Logical OR Operator (One True)
TEST_F(VmTest, LogicalOrOneTrue) {
    auto result = _vm->exec(R"(
        var a = false;
        var b = true;
        var c = a || b;
        c;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Expected result to be a boolean.";
    EXPECT_EQ(AS_BOOL(result), true) << "Expected c to be true.";
}

// Test 76: Short-Circuit Evaluation of Logical AND
TEST_F(VmTest, LogicalAndShortCircuit) {
    auto result = _vm->exec(R"(
        var a = false;
        var b = (a && (1 / 0 > 0)); // Should not evaluate (1 / 0 > 0)
        b;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Expected result to be a boolean.";
    EXPECT_EQ(AS_BOOL(result), false) << "Expected b to be false.";
}

// Test 77: Short-Circuit Evaluation of Logical OR
TEST_F(VmTest, LogicalOrShortCircuit) {
    auto result = _vm->exec(R"(
        var a = true;
        var b = (a || (1 / 0 > 0)); // Should not evaluate (1 / 0 > 0)
        b;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Expected result to be a boolean.";
    EXPECT_EQ(AS_BOOL(result), true) << "Expected b to be true.";
}

// Test 78: Complex Logical Expression
TEST_F(VmTest, ComplexLogicalExpression) {
    auto result = _vm->exec(R"(
        var x = 5;
        var y = 10;
        var z = !(x > y) && (x < y) || false;
        z;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Expected result to be a boolean.";
    EXPECT_EQ(AS_BOOL(result), true) << "Expected z to be true.";
}

// Test 79: Logical Operators with Non-Boolean Values
TEST_F(VmTest, LogicalOperatorsWithNonBooleanValues) {
    auto result = _vm->exec(R"(
        var a = 0;
        var b = "hello";
        var c = a || b;
        c;
    )");

    // Assuming that 0 is false, "hello" is truthy
    EXPECT_TRUE(IS_STRING(result)) << "Expected result to be a string.";
    EXPECT_EQ(AS_CPP_STRING(result), "hello") << "Expected c to be 'hello'.";
}

// Test 80: Logical NOT with Non-Boolean Value
TEST_F(VmTest, LogicalNotWithNonBooleanValue) {
    auto result = _vm->exec(R"(
        var a = 0;
        var b = !a;
        b;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Expected result to be a boolean.";
    EXPECT_EQ(AS_BOOL(result), true) << "Expected b to be true.";
}


TEST_F(VmTest, SimpleWhileLoopSum) {
    auto result = _vm->exec(R"(
        var i = 0;
        var sum = 0;
        while (i < 10) {
            sum = sum + i;
            i = i + 1;
        }
        sum;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 45);
}

/*// Test while loop with a break condition (since 'break' is not implemented, this will test proper loop exit)
TEST_F(VmTest, WhileLoopWithCondition) {
    auto result = _vm->exec(R"(
        var i = 0;
        while (true) {
            if (i == 5) {
                i = 10; // Simulate break by setting i to exit condition
            }
            i = i + 1;
            if (i >= 10) {
                break; // Since 'break' is not implemented, this will cause an error
            }
        }
        i;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 10);
}*/

// Test nested while loops
TEST_F(VmTest, NestedWhileLoops) {
    auto result = _vm->exec(R"(
        var i = 0;
        var total = 0;
        while (i < 3) {
            var j = 0;
            while (j < 3) {
                total = total + (i * 3 + j);
                j = j + 1;
            }
            i = i + 1;
        }
        total;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 36);
}

// Test while loop that should not execute (condition false at start)
TEST_F(VmTest, WhileLoopNoExecution) {
    auto result = _vm->exec(R"(
        var i = 0;
        while (i < 0) {
            i = i + 1;
        }
        i;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 0);
}

// Test while loop with complex condition
TEST_F(VmTest, WhileLoopComplexCondition) {
    auto result = _vm->exec(R"(
        var i = 0;
        var sum = 0;
        while ((i < 5) && (sum < 10)) {
            sum = sum + i;
            i = i + 1;
        }
        sum;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 10);
}

// Test while loop with logical not
TEST_F(VmTest, WhileLoopWithLogicalNot) {
    auto result = _vm->exec(R"(
        var i = 0;
        while (!(i >= 5)) {
            i = i + 1;
        }
        i;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 5);
}


// Test while loop modifying a global variable
TEST_F(VmTest, WhileLoopGlobalVariable) {
    auto result = _vm->exec(R"(
        var count = 0;
        while (count < 3) {
            count = count + 1;
        }
        count;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 3);
}

// Test while loop with a decrementing counter
TEST_F(VmTest, WhileLoopDecrementCounter) {
    auto result = _vm->exec(R"(
        var i = 5;
        var product = 1;
        while (i > 0) {
            product = product * i;
            i = i - 1;
        }
        product;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 120); // 5!
}

// Test while loop using variables declared inside the loop
TEST_F(VmTest, WhileLoopInnerVariable) {
    auto result = _vm->exec(R"(
        var total = 0;
        var i = 0;
        while (i < 3) {
            var j = i * 2;
            total = total + j;
            i = i + 1;
        }
        total;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 6);
}

TEST_F(VmTest, SimpleForLoopSum) {
    auto result = _vm->exec(R"(
        var sum = 0;
        for (var i = 0; i < 10; i = i + 1) {
            sum = sum + i;
        }
        sum;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 45); // 0 + 1 + 2 + ... + 9 = 45
}

TEST_F(VmTest, ForLoopWithExistingVariable) {
    auto result = _vm->exec(R"(
        var i = 0;
        var sum = 0;
        for (; i < 5; i = i + 1) {
            sum = sum + i;
        }
        sum;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 10); // 0 + 1 + 2 + 3 + 4 = 10
}

TEST_F(VmTest, ForLoopNoInitialization) {
    auto result = _vm->exec(R"(
        var i = 0;
        var sum = 0;
        for (; i < 3; i = i + 1) {
            sum = sum + i;
        }
        sum;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 3); // 0 + 1 + 2 = 3
}

TEST_F(VmTest, ForLoopNoUpdate) {
    auto result = _vm->exec(R"(
        var sum = 0;
        for (var i = 0; i < 5;) {
            sum = sum + i;
            i = i + 1;
        }
        sum;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 10); // 0 + 1 + 2 + 3 + 4 = 10
}


TEST_F(VmTest, NestedForLoops) {
    auto result = _vm->exec(R"(
        var total = 0;
        for (var i = 0; i < 3; i = i + 1) {
            for (var j = 0; j < 3; j = j + 1) {
                total = total + (i * 3 + j);
            }
        }
        total;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 36);
}

TEST_F(VmTest, ForLoopComplexCondition) {
    auto result = _vm->exec(R"(
        var sum = 0;
        for (var i = 0; (i < 5) && (sum < 10); i = i + 1) {
            sum = sum + i;
        }
        sum;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 10);
}

//todo implement support for empty body
/*TEST_F(VmTest, ForLoopWithLogicalNotCondition) {
    auto result = _vm->exec(R"(
        var i = 0;
        for (; !(i >= 5); i = i + 1) {
            // No operation inside the loop
        }
        i;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 5);
}*/


TEST_F(VmTest, ForLoopFactorial) {
    auto result = _vm->exec(R"(
        var factorial = 1;
        for (var i = 1; i <= 5; i = i + 1) {
            factorial = factorial * i;
        }
        factorial;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 120); // 1 * 2 * 3 * 4 * 5 = 120
}
