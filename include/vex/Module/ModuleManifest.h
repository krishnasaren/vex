#pragma once
// include/vex/Module/ModuleManifest.h



// vex/Module/ModuleManifest.h  — Parsed vex.toml project manifest
#include <string>
#include <vector>
#include <unordered_map>

namespace vex {

struct ManifestDependency {
    std::string name;
    std::string version;
    std::string path;   // local path override
    bool        optional = false;
};

struct ModuleManifest {
    std::string name;
    std::string version;
    std::string description;
    std::string author;
    std::string license;
    std::string entryPoint;   // default: "src/main.vex"
    std::string edition;      // language edition "2024"

    std::vector<ManifestDependency>         dependencies;
    std::vector<ManifestDependency>         devDependencies;
    std::unordered_map<std::string,std::string> metadata;
};
} // namespace vex
