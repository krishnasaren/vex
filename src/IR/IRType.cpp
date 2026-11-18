// src/IR/IRType.cpp — IRType factory implementations
#include "vex/IR/IRType.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

namespace vex {

// ── Mutable builder subclass used only inside this TU ────────────────────────
struct IRTypeMut : IRType {
    IRTypeMut() = default;

    void setKind(IRTypeKind k) { kind_ = k; }
    void setWidth(uint32_t w)  { intWidth_ = w; }
    void setSigned(bool s)     { isSigned_ = s; }
    void setName(std::string n){ name_ = std::move(n); }
    void setArrayCount(uint64_t c) { arrayCount_ = c; }
    void setLanes(uint32_t l)  { vectorLanes_ = l; }
    void addSub(IRType* s) { sub_.push_back(s); }
};

static std::unordered_map<std::string, std::unique_ptr<IRTypeMut>> typeCache;

static IRTypeMut* findOrCreate(const std::string& key) {
    auto it = typeCache.find(key);
    if (it != typeCache.end()) return it->second.get();
    auto t = std::make_unique<IRTypeMut>();
    auto* ptr = t.get();
    typeCache[key] = std::move(t);
    return ptr;
}

IRType* IRType::getVoid() {
    auto* t = findOrCreate("void");
    t->setKind(IRTypeKind::Void);
    return t;
}

IRType* IRType::getBool() {
    auto* t = findOrCreate("bool");
    t->setKind(IRTypeKind::Bool);
    t->setWidth(1); t->setSigned(false);
    return t;
}

IRType* IRType::getInt(uint32_t bits, bool isSigned) {
    std::string k = (isSigned ? "i" : "u") + std::to_string(bits);
    auto* t = findOrCreate(k);
    IRTypeKind kind;
    if (bits == 1)        kind = IRTypeKind::Int1;
    else if (bits == 8)   kind = isSigned ? IRTypeKind::Int8   : IRTypeKind::UInt8;
    else if (bits == 16)  kind = isSigned ? IRTypeKind::Int16  : IRTypeKind::UInt16;
    else if (bits == 32)  kind = isSigned ? IRTypeKind::Int32  : IRTypeKind::UInt32;
    else if (bits == 64)  kind = isSigned ? IRTypeKind::Int64  : IRTypeKind::UInt64;
    else if (bits == 128) kind = isSigned ? IRTypeKind::Int128 : IRTypeKind::UInt64;
    else                  kind = isSigned ? IRTypeKind::Int64  : IRTypeKind::UInt64;
    t->setKind(kind); t->setWidth(bits); t->setSigned(isSigned);
    return t;
}

IRType* IRType::getFloat(uint32_t bits) {
    std::string k = "f" + std::to_string(bits);
    auto* t = findOrCreate(k);
    t->setKind(bits == 32 ? IRTypeKind::Float32 : IRTypeKind::Float64);
    t->setWidth(bits); t->setSigned(false);
    return t;
}

IRType* IRType::getPointer(IRType*) {
    auto* t = findOrCreate("ptr");
    t->setKind(IRTypeKind::Pointer); t->setWidth(64); t->setSigned(false);
    return t;
}

IRType* IRType::getArray(IRType* elem, uint64_t count) {
    std::string k = "arr_" + std::to_string((uintptr_t)elem) + "_" + std::to_string(count);
    if (!typeCache.count(k)) {
        auto t = std::make_unique<IRTypeMut>();
        t->setKind(IRTypeKind::Array);
        t->setArrayCount(count);
        t->addSub(elem);
        typeCache[k] = std::move(t);
    }
    return typeCache[k].get();
}

IRType* IRType::getStruct(std::vector<IRType*> fields, std::string name) {
    std::string k = "struct_" + name + "_" + std::to_string(fields.size());
    if (!typeCache.count(k)) {
        auto t = std::make_unique<IRTypeMut>();
        t->setKind(IRTypeKind::Struct);
        t->setName(name);
        for (auto* f : fields) {
        /* sub already added above */
        }
        typeCache[k] = std::move(t);
    }
    return typeCache[k].get();
}

IRType* IRType::getFunction(IRType* ret, std::vector<IRType*> params, bool) {
    std::string k = "fn_" + std::to_string((uintptr_t)ret);
    for (auto* p : params) k += "_" + std::to_string((uintptr_t)p);
    if (!typeCache.count(k)) {
        auto t = std::make_unique<IRTypeMut>();
        t->setKind(IRTypeKind::Function);
        t->addSub(ret);
        for (auto* p : params) {
            t->addSub(p);
        }
        typeCache[k] = std::move(t);
    }
    return typeCache[k].get();
}

IRType* IRType::getVector(IRType* elem, uint32_t lanes) {
    std::string k = "vec_" + std::to_string(lanes) + "_" + std::to_string((uintptr_t)elem);
    if (!typeCache.count(k)) {
        auto t = std::make_unique<IRTypeMut>();
        t->setKind(IRTypeKind::FixedVector);
        t->setLanes(lanes);
        t->addSub(elem);
        typeCache[k] = std::move(t);
    }
    return typeCache[k].get();
}

IRType* IRType::make_() {
    auto t = std::make_unique<IRTypeMut>();
    auto* ptr = t.get();
    std::string k = "tmp_" + std::to_string((uintptr_t)ptr);
    typeCache[k] = std::move(t);
    return ptr;
}

uint32_t IRType::bitWidth() const {
    if (intWidth_) return intWidth_;
    switch (kind_) {
    case IRTypeKind::Bool:    return 1;
    case IRTypeKind::Int1:    return 1;
    case IRTypeKind::Int8:    return 8;    case IRTypeKind::UInt8:  return 8;
    case IRTypeKind::Int16:   return 16;   case IRTypeKind::UInt16: return 16;
    case IRTypeKind::Int32:   return 32;   case IRTypeKind::UInt32: return 32;
    case IRTypeKind::Int64:   return 64;   case IRTypeKind::UInt64: return 64;
    case IRTypeKind::Int128:  return 128;
    case IRTypeKind::Float32: return 32;
    case IRTypeKind::Float64: return 64;
    case IRTypeKind::Float128:return 128;
    case IRTypeKind::Pointer: return 64;
    default: return 0;
    }
}

std::string IRType::toString() const {
    switch (kind_) {
    case IRTypeKind::Void:    return "void";
    case IRTypeKind::Bool:    return "bool";
    case IRTypeKind::Int1:    return "i1";
    case IRTypeKind::Int8:    return "i8";    case IRTypeKind::UInt8:  return "u8";
    case IRTypeKind::Int16:   return "i16";   case IRTypeKind::UInt16: return "u16";
    case IRTypeKind::Int32:   return "i32";   case IRTypeKind::UInt32: return "u32";
    case IRTypeKind::Int64:   return "i64";   case IRTypeKind::UInt64: return "u64";
    case IRTypeKind::Int128:  return "i128";
    case IRTypeKind::Float32: return "f32";
    case IRTypeKind::Float64: return "f64";
    case IRTypeKind::Pointer: return "ptr";
    case IRTypeKind::Struct:  return "struct{" + name_ + "}";
    case IRTypeKind::Array:   return "array[" + std::to_string(arrayCount_) + "]";
    default: return "?";
    }
}

} // namespace vex
