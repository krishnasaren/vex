#pragma once
// include/vex/Core/ADT/StringMap.h



// vex/Core/ADT/StringMap.h  — Alias to llvm::StringMap
#include "llvm/ADT/StringMap.h"
namespace vex {
    template<typename V>
    using StringMap = llvm::StringMap<V>;
}

