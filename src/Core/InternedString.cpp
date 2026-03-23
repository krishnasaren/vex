// src/Core/InternedString.cpp



#include "vex/Core/InternedString.h"
#include <cstring>

namespace vex {

InternedString InternedString::empty_str() {
    static const char empty = '\0';
    return InternedString(&empty, 0);
}

} // namespace vex
