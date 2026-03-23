#pragma once
// include/vex/Driver/PipelineStage.h

// vex/Driver/PipelineStage.h  — pipeline stage enum for -stop-after=
#include <string_view>
namespace vex {
enum class PipelineStage : uint8_t {
    Lex, Parse, Sema, IRGen, Optimize, CodeGen, Link
};
PipelineStage pipelineStageFromString(std::string_view s);
} // namespace vex
