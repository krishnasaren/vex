#pragma once
// include/vex/Core/Error.h



#pragma once
// ============================================================================
// vex/Core/Error.h  — Error / Result types for the compiler pipeline
// ============================================================================
#include <string>
#include <string_view>
#include <variant>

namespace vex {

// A simple compiler error
struct Error {
    enum class Kind : uint8_t {
        Internal, Parse, Type, Borrow, Name, IO, Linker, Generic
    };
    Kind        kind    = Kind::Generic;
    std::string message;

    static Error internal(std::string_view msg)  { return {Kind::Internal, std::string(msg)}; }
    static Error io(std::string_view msg)         { return {Kind::IO,       std::string(msg)}; }
    static Error generic(std::string_view msg)    { return {Kind::Generic,  std::string(msg)}; }
};

template <typename T>
class Result {
public:
    static Result ok(T val)     { Result r; r.value_  = std::move(val);  r.ok_ = true;  return r; }
    static Result err(Error e)  { Result r; r.error_  = std::move(e);    r.ok_ = false; return r; }

    bool   isOk()    const { return ok_; }
    bool   isErr()   const { return !ok_; }

    const T&     value() const { return value_; }
    T&&          take()        { return std::move(value_); }
    const Error& error() const { return error_; }

    explicit operator bool() const { return ok_; }

private:
    bool  ok_ = false;
    T     value_;
    Error error_;
};

template<>
class Result<void> {
public:
    static Result ok()          { Result r; r.ok_ = true;  return r; }
    static Result err(Error e)  { Result r; r.error_ = std::move(e); r.ok_ = false; return r; }

    bool   isOk()  const { return ok_; }
    bool   isErr() const { return !ok_; }
    const  Error& error() const { return error_; }
    explicit operator bool() const { return ok_; }

private:
    bool  ok_ = false;
    Error error_;
};

using VoidResult = Result<void>;

} // namespace vex


