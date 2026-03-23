#pragma once
// include/vex/Core/ADT/SmallVector.h



// vex/Core/ADT/SmallVector.h  — Alias to llvm::SmallVector
#include "llvm/ADT/SmallVector.h"
namespace vex {
    template<typename T, unsigned N = 8>
    using SmallVector = llvm::SmallVector<T, N>;
}






