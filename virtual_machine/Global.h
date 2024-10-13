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
            DIE << "Global " << index << " doesn't exist.";
        }
        globals[index].value = value;
    }

    void define(const std::string& name) {
        auto index = getGlobalIndex(name);

        if (index != -1) {
            // It's already defined
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
        if (globals.size() > 0) {
            for (auto i = (int)globals.size() - 1; i >= 0; i--) {
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
