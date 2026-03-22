// ============================================================================
// vex/Core/VexContext.cpp
// ============================================================================

#include "vex/Core/VexContext.h"

namespace vex {

VexContext::VexContext(DiagnosticConsumer& consumer,
                       CompilerOptions     opts)
    : opts_(std::move(opts))
    , srcMgr_()
    , diagEngine_(consumer)
{
    diagEngine_.setWarningsAsErrors(opts_.warningsAsErrors);
    diagEngine_.setSuppressWarnings(false);
}

} // namespace vex