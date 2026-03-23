#pragma once
// include/vex/Module/PackageResolver.h




// vex/Module/PackageResolver.h  — Resolves package dependencies from vex.toml
#include "vex/Module/ModuleManifest.h"
#include "vex/Core/DiagnosticEngine.h"
#include <string>
#include <string_view>
#include <vector>

namespace vex {

struct ResolvedPackage {
    std::string name;
    std::string version;
    std::string rootPath;
};

class PackageResolver {
public:
    explicit PackageResolver(DiagnosticEngine& diags) : diags_(diags) {}

    void addRegistryPath(std::string_view path);

    // Resolve all dependencies from a manifest
    std::vector<ResolvedPackage> resolve(const ModuleManifest& manifest);

    // Locate the root path for a package
    std::string locatePackage(std::string_view name, std::string_view version);

private:
    DiagnosticEngine&        diags_;
    std::vector<std::string> registryPaths_;
};
} // namespace vex
