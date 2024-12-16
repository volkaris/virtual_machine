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

struct Global{
    GlobalVar& get(size_t index) { return globals[index]; }

    void set(size_t index, const EvaluationValue& value) {
        if (index >= globals.size()) {
            throw std::runtime_error("Global " + std::to_string(index) + " doesn't exist.");

        }
        globals[index].value = value;
    }


 void    addNativeFunction(const std::string& name,std::function<void()> fn,size_t arity) {
        if (exists(name)) {
            return;
        }
        globals.push_back({name,ALLOC_NATIVE(fn,name,arity)});

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
            for (auto i = static_cast<int>(globals.size()) - 1; i >= 0; i--) {
                if (globals[i].name == name) {
                    return i;
                }
            }
        }
        return -1;
    }

    bool exists(const std::string& name) { return getGlobalIndex(name) != -1; }

    std::vector<GlobalVar> globals;
};
