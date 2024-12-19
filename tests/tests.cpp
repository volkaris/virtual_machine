
#include <gtest/gtest.h>
#include "vm.h"


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


TEST_F(VmTest, IfConditionFalse) {
    auto result = _vm->exec(R"(
        if (5 > 10) {1;} else {2;}
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 2) << "Ожидалось значение 2.";
}


TEST_F(VmTest, IfConditionTrue) {
    auto result = _vm->exec(R"(
        if (10 > 5) {1;} else {2;}
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 1) << "Ожидалось значение 1.";
}


TEST_F(VmTest, NestedIfExpressions) {
    auto result = _vm->exec(R"(
        if (5 > 3) {
            if (2 > 1) {100;} else {200;} }
else {
            if (3 > 2) {300;} else {400;}
        }
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 100) << "Ожидалось значение 100.";
}


TEST_F(VmTest, MultipleIfExpressions) {
    auto result1 = _vm->exec(R"(
        if (1 == 1) {10;} else { 20;}
    )");
    auto result2 = _vm->exec(R"(
        if(2 == 3) {30;} else{40;}
    )");

    EXPECT_TRUE(IS_NUMBER(result1)) << "Ожидалось число для result1.";
    EXPECT_EQ(AS_NUMBER(result1), 10) << "Ожидалось значение 10 для result1.";

    EXPECT_TRUE(IS_NUMBER(result2)) << "Ожидалось число для result2.";
    EXPECT_EQ(AS_NUMBER(result2), 40) << "Ожидалось значение 40 для result2.";
}


TEST_F(VmTest, ComplexExpressions) {
    auto result = _vm->exec(R"(
        if ((5 + 5) == (2 * 5)) {
            10 * 10;}
else {
            20 * 20;
      }
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 100) << "Ожидалось значение 100.";
}


TEST_F(VmTest, IfWithBooleanLiterals) {
    auto result = _vm->exec(R"(
        if (true) {1;}  else {2;}
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 1) << "Ожидалось значение 1.";
}


TEST_F(VmTest, IfWithStringComparison) {
    auto result = _vm->exec(R"(
        if ("hello" == "hello") {"yes";} else{"no";}
    )");

    EXPECT_TRUE(IS_STRING(result)) << "Ожидалась строка.";
    EXPECT_EQ(AS_CPP_STRING(result), "yes") << "Ожидалось значение 'yes'.";
}


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

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 3) << "Ожидалось значение 3.";
}


TEST_F(VmTest, DivisionByZero) {
    EXPECT_THROW({
                 _vm->exec("(5 / 0);");
                 }, std::exception);
}


TEST_F(VmTest, SimpleAddition) {
    auto result = _vm->exec("(5 + 10);");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 15) << "Ожидалось значение 15.";
}


TEST_F(VmTest, StringConcatenation) {
    auto result = _vm->exec(R"(
        ("Hello" + " World");
    )");

    EXPECT_TRUE(IS_STRING(result)) << "Ожидалась строка.";
    EXPECT_EQ(AS_CPP_STRING(result), "Hello World") << "Ожидалось значение 'Hello World'.";
}


TEST_F(VmTest, InvalidOperationStringNumber) {
    EXPECT_THROW({
                 _vm->exec(R"(
            ("Hello" + 5);
        )");
                 }, std::exception);
}


TEST_F(VmTest, GreaterThanComparison) {
    auto result = _vm->exec("(5 > 3);");

    EXPECT_TRUE(IS_BOOL(result)) << "Ожидалось булево значение.";
    EXPECT_TRUE(AS_BOOL(result)) << "Ожидалось true для 5 > 3.";
}


TEST_F(VmTest, LessThanComparison) {
    auto result = _vm->exec("(2 < 4);");

    EXPECT_TRUE(IS_BOOL(result)) << "Ожидалось булево значение.";
    EXPECT_TRUE(AS_BOOL(result)) << "Ожидалось true для 2 < 4.";
}


TEST_F(VmTest, EqualityComparison) {
    auto result = _vm->exec("(5 == 5);");

    EXPECT_TRUE(IS_BOOL(result)) << "Ожидалось булево значение.";
    EXPECT_TRUE(AS_BOOL(result)) << "Ожидалось true для 5 == 5.";
}


TEST_F(VmTest, GreaterThanOrEqualComparison) {
    auto result = _vm->exec("(5 >= 5);");

    EXPECT_TRUE(IS_BOOL(result)) << "Ожидалось булево значение.";
    EXPECT_TRUE(AS_BOOL(result)) << "Ожидалось true для 5 >= 5.";
}


TEST_F(VmTest, LessThanOrEqualComparison) {
    auto result = _vm->exec("(3 <= 5);");

    EXPECT_TRUE(IS_BOOL(result)) << "Ожидалось булево значение.";
    EXPECT_TRUE(AS_BOOL(result)) << "Ожидалось true для 3 <= 5.";
}


TEST_F(VmTest, NotEqualComparison) {
    auto result = _vm->exec("(5 != 3);");

    EXPECT_TRUE(IS_BOOL(result)) << "Ожидалось булево значение.";
    EXPECT_TRUE(AS_BOOL(result)) << "Ожидалось true для 5 != 3.";
}


TEST_F(VmTest, InvalidComparison) {
    auto result = _vm->exec("(5 < 3);");

    EXPECT_TRUE(IS_BOOL(result)) << "Ожидалось булево значение.";
    EXPECT_FALSE(AS_BOOL(result)) << "Ожидалось false для 5 < 3.";
}


TEST_F(VmTest, ChainedOperations) {
    auto result = _vm->exec("((5 + 10) * (2 + 3));");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 75) << "Ожидалось значение 75.";
}


TEST_F(VmTest, DeeplyNestedExpressions) {
    auto result = _vm->exec("(((1 + 2) + 3) + 4);");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 10) << "Ожидалось значение 10.";
}


TEST_F(VmTest, DeconstructProgramByAssem) {
    auto result = _vm->exec("10;");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 10) << "Ожидалось значение 10.";
}


TEST_F(VmTest, DeconstructSum) {
    auto result = _vm->exec("(5 + 2);");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 7) << "Ожидалось значение 7.";
}


TEST_F(VmTest, DeconstructDifference) {
    auto result = _vm->exec("if (5 > 10) {1;} else{2;}");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 2) << "Ожидалось значение 2.";
}


TEST_F(VmTest, VariableDeclarationAndUsage) {
    auto result = _vm->exec(R"(
        var x = 10;
        var y = 20;
        if (x > y) {1;} {2;}
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 2) << "Ожидалось значение 2.";
}


TEST_F(VmTest, VariableModificationAndAccess) {
    auto result = _vm->exec(R"(
        var x = 5;
        x = x + 10;
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 15) << "Ожидалось значение 15.";
}


TEST_F(VmTest, UndefinedVariableAccess) {
    EXPECT_THROW({
                 _vm->exec(R"(
            var x = 10;
            y = x + 5; // 'y' не объявлена
        )");
                 }, std::exception);
}


TEST_F(VmTest, AssignmentBeforeDeclaration) {
    EXPECT_THROW({
                 _vm->exec(R"(
            a = 10; // 'a' ещё не объявлена
            var a = 5;
        )");
                 }, std::exception);
}


TEST_F(VmTest, ChainedVariableUsage) {
    auto result = _vm->exec(R"(
        var a = 2;
        var b = a * 3;
        var c = b + a;
        c;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 8) << "Ожидалось значение 8.";
}


TEST_F(VmTest, RedeclarationOfVariable) {
    EXPECT_THROW({
                 _vm->exec(R"(
            var x = 10;
            var x = 20; // Повторное объявление 'x'
        )");
                 }, std::exception);
}


TEST_F(VmTest, UsingVariableBeforeDeclaration) {
    EXPECT_THROW({
                 _vm->exec(R"(
            y = 10; // 'y' не объявлена
            var y = 5;
        )");
                 }, std::exception);
}


TEST_F(VmTest, OuterScopeVariablesAccessibleInInnerScope) {
    auto result = _vm->exec(R"(
        var x = 10;
        var y = 0;
        {
            y = x + 5; // использование внешней переменной x
        }
        y; // Должно быть 15
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 15) << "Ожидалось значение 15.";
}


TEST_F(VmTest, VariableShadowing) {
    auto result = _vm->exec(R"(
        var x = 5;
        {
            var x = 10; // Внутренняя x затеняет внешнюю
            x = x + 5;  // Внутренняя x = 15
        }
        x; // Должно остаться 5
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 5) << "Ожидалось значение 5.";
}


TEST_F(VmTest, VariablesNotAccessibleOutsideScope) {
    EXPECT_THROW({
                 _vm->exec(R"(
            {
                var x = 10;
            }
            x; // x недоступна здесь
        )");
                 }, std::exception);
}


TEST_F(VmTest, AssignmentToOuterScopeVariableFromInnerScope) {
    auto result = _vm->exec(R"(
        var x = 10;
        {
            x = x + 5; // Изменяем внешнюю x
        }
        x; // Должно быть 15
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 15) << "Ожидалось значение 15.";
}


TEST_F(VmTest, NestedScopesWithVariableDeclarations) {
    auto result = _vm->exec(R"(
        var x = 1;
        {
            var y = 2;
            {
                var z = x + y; // z = 3
            }
            // z недоступна здесь
            y = y + x; // y = 3
        }
        x; // Должно быть 1
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 1) << "Ожидалось значение 1.";
}


TEST_F(VmTest, VariableLifetimeEndsWithScope) {
    EXPECT_THROW({
                 auto result = _vm->exec(R"(
            {
                var x = 10;
            }
            x = 20; // Ошибка, x вне области видимости
        )");
                 }, std::exception);
}


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
            x = x + y; // x = 9
        }
        x + y; // Внешний x = 1, y = 4, ожидается 5
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 5) << "Ожидалось значение 5.";
}


TEST_F(VmTest, RedeclaringVariableInSameScope) {
    EXPECT_THROW({
                 _vm->exec(R"(
            var x = 10;
            var x = 20; // Повторное объявление
        )");
                 }, std::exception);
}


TEST_F(VmTest, SimpleIfStatementTrue) {
    auto result = _vm->exec(R"(
        var x = 5;
        if (x == 5) {
            x = x + 1;
        }
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 6) << "Ожидалось значение 6.";
}

TEST_F(VmTest, SimpleIfStatementFalse) {
    auto result = _vm->exec(R"(
        var x = 5;
        if (x != 5) {
            x = x + 1;
        }
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 5) << "Ожидалось значение 5.";
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

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 6) << "Ожидалось значение 6.";
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

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 4) << "Ожидалось значение 4.";
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

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 2) << "Ожидалось значение 2.";
}

TEST_F(VmTest, IfWithoutBraces) {
    auto result = _vm->exec(R"(
        var x = 5;
        if (x == 5)
            x = x + 1;
        x;
    )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 6) << "Ожидалось значение 6.";
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

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 4) << "Ожидалось значение 4.";
}

// Важный тест!!! Правильная ассоциация "висячего" else
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

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 4) << "Ожидалось значение 4.";
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

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 10) << "Ожидалось значение 10.";
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

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 6) << "Ожидалось значение 6.";
}

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

    EXPECT_TRUE(IS_BOOL(result)) << "Ожидалось булево значение.";
    EXPECT_EQ(AS_BOOL(result), false) << "Ожидалось значение false.";
}


TEST_F(VmTest, LogicalAndBothTrue) {
    auto result = _vm->exec(R"(
        var a = true;
        var b = true;
        var c = a && b;
        c;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Ожидалось булево значение.";
    EXPECT_EQ(AS_BOOL(result), true) << "Ожидалось true.";
}


TEST_F(VmTest, LogicalAndOneFalse) {
    auto result = _vm->exec(R"(
        var a = true;
        var b = false;
        var c = a && b;
        c;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Ожидалось булево значение.";
    EXPECT_EQ(AS_BOOL(result), false) << "Ожидалось false.";
}


TEST_F(VmTest, LogicalOrBothFalse) {
    auto result = _vm->exec(R"(
        var a = false;
        var b = false;
        var c = a || b;
        c;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Ожидалось булево значение.";
    EXPECT_EQ(AS_BOOL(result), false) << "Ожидалось false.";
}


TEST_F(VmTest, LogicalOrOneTrue) {
    auto result = _vm->exec(R"(
        var a = false;
        var b = true;
        var c = a || b;
        c;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Ожидалось булево значение.";
    EXPECT_EQ(AS_BOOL(result), true) << "Ожидалось true.";
}


TEST_F(VmTest, LogicalAndShortCircuit) {
    auto result = _vm->exec(R"(
        var a = false;
        var b = (a && (1 / 0 > 0)); // Не должно вычисляться (1 / 0 > 0)
        b;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Ожидалось булево значение.";
    EXPECT_EQ(AS_BOOL(result), false) << "Ожидалось false.";
}


TEST_F(VmTest, LogicalOrShortCircuit) {
    auto result = _vm->exec(R"(
        var a = true;
        var b = (a || (1 / 0 > 0)); // Не должно вычисляться (1 / 0 > 0)
        b;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Ожидалось булево значение.";
    EXPECT_EQ(AS_BOOL(result), true) << "Ожидалось true.";
}


TEST_F(VmTest, ComplexLogicalExpression) {
    auto result = _vm->exec(R"(
        var x = 5;
        var y = 10;
        var z = !(x > y) && (x < y) || false;
        z;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Ожидалось булево значение.";
    EXPECT_EQ(AS_BOOL(result), true) << "Ожидалось true.";
}


TEST_F(VmTest, LogicalOperatorsWithNonBooleanValues) {
    auto result = _vm->exec(R"(
        var a = 0;
        var b = "hello";
        var c = a || b;
        c;
    )");

    // Предполагается, что 0 - ложь, "hello" - истинно
    EXPECT_TRUE(IS_STRING(result)) << "Ожидалась строка.";
    EXPECT_EQ(AS_CPP_STRING(result), "hello") << "Ожидалось 'hello'.";
}


TEST_F(VmTest, LogicalNotWithNonBooleanValue) {
    auto result = _vm->exec(R"(
        var a = 0;
        var b = !a;
        b;
    )");

    EXPECT_TRUE(IS_BOOL(result)) << "Ожидалось булево значение.";
    EXPECT_EQ(AS_BOOL(result), true) << "Ожидалось true.";
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
    EXPECT_EQ(AS_NUMBER(result), 45);
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
    EXPECT_EQ(AS_NUMBER(result), 10);
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
    EXPECT_EQ(AS_NUMBER(result), 3);
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
    EXPECT_EQ(AS_NUMBER(result), 10);
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

TEST_F(VmTest, BasicFunctionCall) {
    auto result = _vm->exec (R"(
        func add(a, b) {
            return a + b;
        }
        add(3, 4);
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 7);
}

TEST_F(VmTest, RecursiveFactorial) {
    auto result = _vm->exec ( R"(
       var number = 5;

func factorial(n) {
    if (n == 0) {
        return 1;
    }
    else {
        return n * factorial(n - 1);
    }
}

var fact = factorial(number);

fact;
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 120);
}

TEST_F(VmTest, RecursiveFactorial2) {
    auto result = _vm->exec ( R"(
       func factorial(n) {
    if (n == 0) {
        return 1;
    }
    else {
        return n * factorial(n - 1);
    }
}

factorial(5);
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 120);
}

TEST_F(VmTest, FunctionWithReturnType) {
    auto result = _vm->exec (  R"(
        func multiply(a, b) {
            return a * b;
        }
        multiply(6, 7);
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 42);
}

TEST_F(VmTest, FunctionWithMultipleParameters) {
    auto result = _vm->exec (R"(
        func sum(a, b, c) {
            return a + b + c;
        }
        sum(10, 20, 30);
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 60);
}

TEST_F(VmTest, FunctionWithVariableScoping) {
    auto result = _vm->exec ( R"(
        func scopeTest() {
            var a = 10;
            return a;
        }
        scopeTest();
    )");

    ASSERT_TRUE(IS_NUMBER(result));
    EXPECT_EQ(AS_NUMBER(result), 10);
}

TEST_F(VmTest, ConstantFolding) {
    auto result = _vm->exec(R"( var a = (2+3)*4;
                            a; )");

    EXPECT_TRUE(IS_NUMBER(result)) << "Ожидалось число.";
    EXPECT_EQ(AS_NUMBER(result), 20) << "Ожидалось значение 20.";
}
