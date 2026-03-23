#pragma once
// include/vex/Core/Optional.h


// ============================================================================
// vex/Core/Optional.h  — Thin alias over std::optional with VEX naming
// ============================================================================
#include <optional>
#include <stdexcept>

namespace vex {

template <typename T>
using Optional = std::optional<T>;

inline constexpr std::nullopt_t None = std::nullopt;

template <typename T>
Optional<T> Some(T val) { return Optional<T>(std::move(val)); }

} // namespace vex

