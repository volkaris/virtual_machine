//
// Created by valer on 13.10.2024.
//
#pragma once
#include <string>
#include <vector>
#include "EvaluationValue.h"

struct GlobalVar {
    std::string name;
    EvaluationValue value;
};

struct Global {
    std::vector<GlobalVar> globals;
    // We can also store functions globally
    // but we will store them in vm or bytecodeGenerator and pass them to vm.

    GlobalVar& get(size_t index) { return globals[index]; }

    void set(size_t index, const EvaluationValue& value) {
        if (index >= globals.size()) {
            throw std::runtime_error("Global index out of range.");
        }
        globals[index].value = value;
    }

    void define(const std::string& name) {
        auto index = getGlobalIndex(name);
        if (index != -1) {
            return;
        }
        globals.push_back({name, NUMBER(0)});
    }

    void addConst(const std::string& name, double value) {
        if (exists(name)) {
            return;
        }
        globals.push_back({name, NUMBER(value)});
    }

    int getGlobalIndex(const std::string& name) {
        if (!globals.empty()) {
            for (int i = (int)globals.size() - 1; i >= 0; i--) {
                if (globals[i].name == name) {
                    return i;
                }
            }
        }
        return -1;
    }

    bool exists(const std::string& name) { return getGlobalIndex(name) != -1; }
};
