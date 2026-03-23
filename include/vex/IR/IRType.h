#pragma once
// include/vex/IR/IRType.h



// ============================================================================
// vex/IR/IRType.h  — Low-level type system for VEX IR (below VEX types)
// ============================================================================
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace vex {

enum class IRTypeKind : uint8_t {
    Void, Bool,
    Int1, Int8, Int16, Int32, Int64, Int128,
    UInt8, UInt16, UInt32, UInt64,
    Float32, Float64, Float128,
    Pointer,
    Array,
    FixedVector,   // SIMD
    Struct,
    Function,
    Label,
    Opaque,        // forward declaration
};

class IRType {
public:
    IRTypeKind kind() const { return kind_; }
    bool isVoid()  const { return kind_ == IRTypeKind::Void; }
    bool isInt()   const { return kind_ >= IRTypeKind::Int1  && kind_ <= IRTypeKind::UInt64; }
    bool isFloat() const { return kind_ >= IRTypeKind::Float32 && kind_ <= IRTypeKind::Float128; }
    bool isPtr()   const { return kind_ == IRTypeKind::Pointer; }
    bool isStruct()const { return kind_ == IRTypeKind::Struct; }
    bool isFn()    const { return kind_ == IRTypeKind::Function; }

    // Numeric bit width (for int types)
    uint32_t bitWidth() const;

    // For pointer types — pointee
    IRType* pointee() const { return sub_.empty() ? nullptr : sub_[0].get(); }

    // For struct/function types — element types
    const std::vector<std::unique_ptr<IRType>>& elements() const { return sub_; }

    std::string toString() const;

    // Factories
    static IRType* getVoid();
    static IRType* getBool();
    static IRType* getInt(uint32_t bits, bool isSigned = true);
    static IRType* getFloat(uint32_t bits);
    static IRType* getPointer(IRType* pointee);
    static IRType* getArray(IRType* elem, uint64_t count);
    static IRType* getStruct(std::vector<IRType*> fields, std::string name = "");
    static IRType* getFunction(IRType* ret, std::vector<IRType*> params, bool vararg = false);
    static IRType* getVector(IRType* elem, uint32_t lanes);

private:
    IRTypeKind kind_;
    uint32_t   intWidth_ = 0;
    uint64_t   arrayCount_ = 0;
    uint32_t   vectorLanes_ = 0;
    std::string name_;
    std::vector<std::unique_ptr<IRType>> sub_;
    bool isSigned_ = true;
};

} // namespace vex
