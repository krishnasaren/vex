
#pragma once
// vex/Optimizer/OptLevel.h
#include <cstdint>
namespace vex {
enum class OptLevel : uint8_t { O0=0, O1=1, O2=2, O3=3, Os=4, Oz=5 };
OptLevel optLevelFromInt(int n);
} // namespace vex







