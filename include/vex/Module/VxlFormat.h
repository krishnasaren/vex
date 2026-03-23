#pragma once
// include/vex/Module/VxlFormat.h



#pragma once
// ============================================================================
// vex/Module/VxlFormat.h  — .vxl (VEX module binary) format definitions
// Magic, version, section types.
// ============================================================================
#include <cstdint>

namespace vex {
namespace vxl {

static constexpr uint32_t MAGIC   = 0x56584C00; // "VXL\0"
static constexpr uint32_t VERSION = 1;

enum class SectionKind : uint8_t {
    Header     = 0,
    AST        = 1,
    Types      = 2,
    Symbols    = 3,
    Exports    = 4,
    Imports    = 5,
    IR         = 6,
    DebugInfo  = 7,
    DocComments= 8,
};

#pragma pack(push,1)
struct VxlHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t moduleNameOffset;
    uint32_t sectionCount;
    uint32_t flags;
    uint64_t buildTimestamp;
};

struct VxlSectionHeader {
    uint8_t  kind;
    uint32_t offset;
    uint32_t size;
    uint32_t checksum;
};
#pragma pack(pop)

} // namespace vxl
} // namespace vex
