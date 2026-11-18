// src/Core/VexContext.cpp
#include "vex/Core/VexContext.h"

namespace vex {

VexContext::VexContext(DiagnosticConsumer& consumer, CompilerOptions opts)
    : opts_(std::move(opts))
    , srcMgr_()
    , diagEngine_(consumer)
{
    if (opts_.warningsAsErrors)
        diagEngine_.setWarningsAsErrors(true);
}

} // namespace vex
