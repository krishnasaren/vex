#pragma once
#include <string>
#include <cstdint>

namespace vex {

struct TargetOptions {
    bool softFloat       = false;
    bool unsafeFPMath    = false;
    bool noInfsFPMath    = false;
    bool noNaNsFPMath    = false;
    bool approxFuncFPMath= false;
    bool noSignedZerosFPMath = false;
    bool enableIPRA      = false;
    bool emitAddrsig     = false;
    bool bbSections      = false;
    bool noRedZone       = false;
    bool stackRealign    = false;
    std::string abi;
};

} // namespace vex
