#pragma once

#include <unordered_map>
#include <functional>
#include <random>
#include <string>
#include <vector>
#include "EvaluationValue.h"

struct GlobalVar {
    std::string name;
    EvaluationValue value;
};


struct Global {
    std::vector<GlobalVar> globals;

    std::mt19937 rng;

    // Таблица встроенных функций: имя функции -> указатель на функцию
    std::unordered_map<std::string, std::function<EvaluationValue(const std::vector<EvaluationValue> &)> >
    builtinFunctions;

    // Метод для регистрации встроенной функции
    void registerBuiltin(const std::string &name,
                         const std::function<EvaluationValue(const std::vector<EvaluationValue> &)> &func) {
        builtinFunctions[name] = func;
    }

    // Метод для определения глобальной переменной
    void define(const std::string &name) {
        if (getGlobalIndex(name) != -1) {
            return; // Уже определена
        }
        globals.push_back({name, NIL()});
    }

    // Метод для получения индекса глобальной переменной
    int getGlobalIndex(const std::string &name) {
        for (int i = 0; i < globals.size(); ++i) {
            if (globals[i].name == name) {
                return i;
            }
        }
        return -1;
    }

    // Метод для проверки существования глобальной переменной
    bool exists(const std::string &name) const {
        for (const auto &var: globals) {
            if (var.name == name) {
                return true;
            }
        }
        return false;
    }

    void setGlobalVariables() {

        registerBuiltin("random", [this](const std::vector<EvaluationValue> &args) -> EvaluationValue {
            initializeRNG();


            if (args.size() != 1) {
                throw std::runtime_error("Функция random ожидает 1 аргумент: max.");
            }
            if (!IS_NUMBER(args[0])) {
                throw std::runtime_error("Функция random принимает только числовой аргумент.");
            }
            double maxVal = AS_NUMBER(args[0]);
            double minVal = -maxVal;

            if (minVal > maxVal) {
                throw std::runtime_error("В функции random min не может быть больше max.");
            }


            std::uniform_real_distribution dist(minVal, maxVal);
            int randomValue = dist(rng);
            return NUMBER(randomValue);
        });



        registerBuiltin("print", [](const std::vector<EvaluationValue>& args) -> EvaluationValue {
            for (const auto& arg : args) {
                std::cout << evaluationValueToConstantString(arg) << " ";
            }
            std::cout << std::endl;
            return NIL();
        });


    }


    // Метод для получения глобальной переменной
    GlobalVar &get(size_t index) {
        if (index >= globals.size()) {
            throw std::runtime_error("Глобальное значение " + std::to_string(index) + " не существует");
        }
        return globals[index];
    }

    // Метод для установки значения глобальной переменной
    void set(size_t index, const EvaluationValue &value) {
        if (index >= globals.size()) {
            throw std::runtime_error("Глобальное значение " + std::to_string(index) + " не существует");
        }
        globals[index].value = value;
    }

    // Метод для вызова встроенной функции
    EvaluationValue callBuiltin(const std::string &name, const std::vector<EvaluationValue> &args) {
        auto it = builtinFunctions.find(name);
        if (it != builtinFunctions.end()) {
            return it->second(args);
        }
        throw std::runtime_error("Встроенная функция не найдена: " + name);
    }

    void initializeRNG() {
        std::random_device rd; // Источник энтропии
        rng.seed(rd());
    }
};
