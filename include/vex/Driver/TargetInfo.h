#pragma once
// include/vex/Driver/TargetInfo.h

// ============================================================================
// vex/Driver/TargetInfo.h  — Target triple, CPU, features
// ============================================================================
#include <string>
#include <string_view>
#include <vector>

namespace vex {

class TargetInfo {
public:
    // Build from explicit triple
    explicit TargetInfo(std::string_view triple);

    // Build from host
    static TargetInfo host();

    // wasm32-unknown-unknown
    static TargetInfo wasm();

    std::string_view triple()    const { return triple_; }
    std::string_view arch()      const { return arch_; }
    std::string_view vendor()    const { return vendor_; }
    std::string_view os()        const { return os_; }
    std::string_view cpu()       const { return cpu_; }

    const std::vector<std::string>& features() const { return features_; }
    void addFeature(std::string_view f) { features_.emplace_back(f); }

    bool isX86_64()  const { return arch_ == "x86_64"; }
    bool isAArch64() const { return arch_ == "aarch64"; }
    bool isWasm()    const { return arch_.starts_with("wasm"); }
    bool isLinux()   const { return os_  == "linux"; }
    bool isWindows() const { return os_  == "windows"; }
    bool isMacOS()   const { return os_  == "darwin" || os_ == "macos"; }

    uint32_t pointerSize()  const { return isWasm() ? 4u : 8u; }
    uint32_t pointerAlign() const { return pointerSize(); }

private:
    std::string              triple_;
    std::string              arch_;
    std::string              vendor_;
    std::string              os_;
    std::string              cpu_;
    std::vector<std::string> features_;

    void parse(std::string_view triple);
};

} // namespace vex
