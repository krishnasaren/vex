#pragma once
// ============================================================================
// vex/Parser/ParseResult.h
// Result type returned by every parse method.
// On success: holds a non-null AST node pointer.
// On failure: null pointer — diagnostic already emitted.
// ============================================================================

#include <cstddef>

namespace vex {

// ── ParseResult<T> ────────────────────────────────────────────────────────────
// T must be a pointer type (AST node pointer).
// Wraps success/failure without requiring exceptions.

template<typename T>
class ParseResult {
public:
    // Success constructor
    explicit ParseResult(T ptr) : ptr_(ptr) {}

    // Failure constructor
    ParseResult(std::nullptr_t) : ptr_(nullptr) {}

    // Implicit conversion from T (allows: return node;)
    ParseResult(T ptr, bool) : ptr_(ptr) {}

    bool ok()     const { return ptr_ != nullptr; }
    bool failed() const { return ptr_ == nullptr; }

    T    get()    const { return ptr_; }
    T    operator->() const { return ptr_; }

    explicit operator bool() const { return ok(); }

    // Release the pointer (caller takes ownership)
    T release() {
        T tmp = ptr_;
        ptr_ = nullptr;
        return tmp;
    }

    static ParseResult<T> error() { return ParseResult<T>(nullptr); }

private:
    T ptr_;
};

// Convenience alias — most parse methods return one of these
// ParseResult<ExprNode*>   parseExpr(...)
// ParseResult<StmtNode*>   parseStmt(...)
// ParseResult<DeclNode*>   parseDecl(...)

} // namespace vex