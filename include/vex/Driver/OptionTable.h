#pragma once
// include/vex/Driver/OptionTable.h

// vex/Driver/OptionTable.h  — Static option descriptor table
#include <string_view>
#include <vector>
namespace vex {
enum class OptionType : uint8_t { Flag, String, StringList, Int };
struct OptionDesc {
    std::string_view longName;
    std::string_view shortName;
    OptionType       type;
    std::string_view helpText;
    std::string_view metavar;
};
const std::vector<OptionDesc>& getOptionTable();
} // namespace vex
