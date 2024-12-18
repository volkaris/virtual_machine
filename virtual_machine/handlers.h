#pragma once
#include <cstdint>
#include <vector>

// Forward declare
class vm;
struct CallFrame;

typedef void (*InstructionHandler)(vm*, CallFrame&, const std::vector<uint8_t>&);

// Extern declaration of handlers array
extern InstructionHandler handlers[0xFF + 1];
