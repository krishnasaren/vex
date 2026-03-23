#pragma once
// include/vex/Core/ADT/DenseMap.h


// vex/Core/ADT/DenseMap.h  — Alias to llvm::DenseMap
#include "llvm/ADT/DenseMap.h"
namespace vex {
    template<typename K, typename V>
    using DenseMap = llvm::DenseMap<K, V>;
}
