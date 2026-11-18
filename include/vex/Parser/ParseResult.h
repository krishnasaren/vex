#pragma once
// vex/Parser/ParseResult.h
#include <cstddef>
#include <type_traits>

namespace vex {

template<typename T>
class ParseResult {
    static_assert(std::is_pointer<T>::value,
                  "ParseResult<T>: T must be a pointer type");
public:
    // Implicit from T (return node;)
    ParseResult(T ptr) : ptr_(ptr) {}           // NOLINT
    // Implicit from nullptr (return nullptr;)
    ParseResult(std::nullptr_t) : ptr_(nullptr) {}  // NOLINT

    bool ok()     const { return ptr_ != nullptr; }
    bool failed() const { return ptr_ == nullptr; }

    T    get()           const { return ptr_; }
    T    operator->()    const { return ptr_; }
    explicit operator bool()  const { return ok(); }

    T release() { T tmp = ptr_; ptr_ = nullptr; return tmp; }

    static ParseResult<T> error() { return nullptr; }

    // Covariant upcast: ParseResult<Derived*> → ParseResult<Base*>
    template<typename U,
             typename std::enable_if<
                 std::is_convertible<T, U>::value, int>::type = 0>
    operator ParseResult<U>() const { return static_cast<U>(ptr_); }

private:
    T ptr_;
};

} // namespace vex
