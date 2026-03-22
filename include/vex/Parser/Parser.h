#pragma once
// ============================================================================
// vex/Parser/Parser.h
// The VEX language parser.
// Uses Pratt parsing for expressions (Chapter 2 precedence rules).
// Uses recursive descent for declarations and statements.
//
// Key rules implemented:
//   - Chapter 2:  operator precedence table
//   - Chapter 8:  function declarations (fn, fn*, async fn, async fn*)
//   - Chapter 9:  lambda syntax |params| -> body
//   - Chapter 10: struct declarations
//   - Chapter 11: class declarations
//   - Chapter 13: trait/interface declarations
//   - Chapter 16: enum declarations
//   - Chapter 17: all control flow statements
//   - Chapter 22: module rules (mod keyword)
//   - Chapter 23: import system (three forms)
// ============================================================================

#include "vex/Parser/ParseResult.h"
#include "vex/Parser/Precedence.h"
#include "vex/Lexer/Token.h"
#include "vex/Core/DiagnosticEngine.h"
#include "vex/Core/SourceManager.h"
#include "vex/AST/ASTContext.h"
#include "vex/AST/Decl.h"
#include "vex/AST/Stmt.h"
#include "vex/AST/Expr.h"
#include "vex/AST/Type.h"
#include "vex/AST/Pattern.h"

#include <vector>
#include <string>

namespace vex {

// ── Parser ────────────────────────────────────────────────────────────────────

class Parser {
public:
    Parser(std::vector<Token>  tokens,
           ASTContext&         ctx,
           SourceManager&      srcMgr,
           DiagnosticEngine&   diags);

    // Parse a complete module (one .vex file) into a list of top-level decls.
    // Returns nullptr on unrecoverable error.
    ModuleDecl* parseModule(std::string_view moduleName);

private:
    std::vector<Token> tokens_;
    size_t             pos_;        // current token index
    ASTContext&        ctx_;
    SourceManager&     srcMgr_;
    DiagnosticEngine&  diags_;

    // ── Token navigation ─────────────────────────────────────────────────────

    Token& current();
    Token& peek(uint32_t offset = 1);
    Token  advance();
    bool   atEnd() const;

    // True if current token has the given kind
    bool check(TokenKind kind) const;

    // Consume current token if it matches kind. Returns true if consumed.
    bool match(TokenKind kind);

    // Consume current token if it matches any of the given kinds.
    template<typename... Kinds>
    bool matchAny(Kinds... kinds) {
        return (match(kinds) || ...);
    }

    // Expect a specific token — emit error and return invalid token if not found.
    // Does NOT consume on failure.
    Token expect(TokenKind kind, std::string_view msg = {});

    // Expect and consume. Returns true on success.
    bool expectConsume(TokenKind kind, std::string_view msg = {});

    SourceLocation currentLoc() const;
    SourceRange    currentRange() const;

    // ── Error recovery ────────────────────────────────────────────────────────

    // Skip tokens until we find one in the sync set or EOF.
    void synchronize(std::initializer_list<TokenKind> syncSet = {});

    // Skip to end of current statement (next ; or })
    void skipToStmtEnd();

    // ── Top-level declarations ────────────────────────────────────────────────

    ParseResult<DeclNode*>      parseTopLevelDecl();
    ParseResult<ImportDecl*>    parseImport();
    ParseResult<FnDecl*>        parseFnDecl(bool isAsync = false,
                                             bool isGenerator = false,
                                             Visibility vis = Visibility::Pub);
    ParseResult<StructDecl*>    parseStructDecl(Visibility vis = Visibility::Pub);
    ParseResult<ClassDecl*>     parseClassDecl(Visibility vis = Visibility::Pub);
    ParseResult<EnumDecl*>      parseEnumDecl(Visibility vis = Visibility::Pub);
    ParseResult<TraitDecl*>     parseTraitDecl(Visibility vis = Visibility::Pub);
    ParseResult<InterfaceDecl*> parseInterfaceDecl(Visibility vis = Visibility::Pub);
    ParseResult<ImplDecl*>      parseImplDecl();
    ParseResult<TypeAliasDecl*> parseTypeAliasDecl(Visibility vis = Visibility::Pub);
    ParseResult<DistinctDecl*>  parseDistinctDecl(Visibility vis = Visibility::Pub);
    ParseResult<VarDecl*>       parseVarDecl(Visibility vis = Visibility::Priv);
    ParseResult<ConstDecl*>     parseConstDecl(Visibility vis = Visibility::Pub);
    ParseResult<BitfieldDecl*>  parseBitfieldDecl(Visibility vis = Visibility::Pub);

    // ── Import parsing (Chapter 22-23) ───────────────────────────────────────

    // Parse import path like std::io or mylib::shapes
    // Returns the path segments (e.g. ["std", "io"])
    std::vector<std::string> parseImportPath();

    // Parse selective import list { name1, name2 as local2 }
    // Returns list of (original_name, local_name) pairs
    struct SelectiveImportItem {
        std::string originalName;
        std::string localName;     // same as originalName if no 'as'
        bool        isWildcard;    // true for { * }
    };
    std::vector<SelectiveImportItem> parseSelectiveImportList();

    // ── Visibility parsing ───────────────────────────────────────────────────

    Visibility parseVisibility();

    // ── Generic parameters <T, U: Bound> ─────────────────────────────────────

    ParseResult<GenericParamList*> parseGenericParams();

    // ── Where clause where T: Bound, U: Bound2 ───────────────────────────────

    ParseResult<WhereClause*> parseWhereClause();

    // ── Function parameter list ───────────────────────────────────────────────

    struct ParsedParam {
        std::string  name;
        TypeNode*    type   = nullptr;
        ExprNode*    defVal = nullptr;  // default value
        bool         isSelf = false;    // self parameter
        bool         isMut  = false;    // &mut T
        bool         isVariadic = false; // ...T
        SourceLocation loc;
    };
    std::vector<ParsedParam> parseParamList();

    // ── Return type annotation ────────────────────────────────────────────────

    TypeNode* parseReturnType(); // -> Type  (optional, nullptr if absent)

    // ── Member declarations (inside struct/class/trait/interface) ─────────────

    ParseResult<DeclNode*>      parseMemberDecl(bool inClass,
                                                 bool inTrait,
                                                 bool inInterface);
    ParseResult<FnDecl*>        parseMemberFn(Visibility vis,
                                               bool isStatic,
                                               bool isAsync,
                                               bool isGenerator,
                                               bool isAbstract);
    ParseResult<PropDecl*>      parsePropDecl(Visibility vis, bool isStatic);
    ParseResult<OperatorDecl*>  parseOperatorDecl(Visibility vis);
    ParseResult<InitDecl*>      parseInitDecl(Visibility vis);
    ParseResult<DeinitDecl*>    parseDeinitDecl();
    ParseResult<FieldDecl*>     parseFieldDecl(Visibility vis, bool isStatic);

    // ── Enum variants ─────────────────────────────────────────────────────────

    ParseResult<EnumVariantDecl*> parseEnumVariant();

    // ── Decorator parsing @decorator ─────────────────────────────────────────

    std::vector<Decorator*> parseDecorators();

    // ── Statement parsing (Chapter 17) ───────────────────────────────────────

    ParseResult<StmtNode*>      parseStmt();
    ParseResult<BlockStmt*>     parseBlock();
    ParseResult<VarDeclStmt*>   parseVarDeclStmt();
    ParseResult<IfStmt*>        parseIfStmt();
    ParseResult<WhileStmt*>     parseWhileStmt();
    ParseResult<DoWhileStmt*>   parseDoWhileStmt();
    ParseResult<ForStmt*>       parseForStmt();
    ParseResult<LoopStmt*>      parseLoopStmt();
    ParseResult<MatchStmt*>     parseMatchStmt();
    ParseResult<ReturnStmt*>    parseReturnStmt();
    ParseResult<SkipStmt*>      parseSkipStmt();   // continue
    ParseResult<StopStmt*>      parseStopStmt();   // break
    ParseResult<DeferStmt*>     parseDeferStmt();
    ParseResult<UseLetStmt*>    parseUseLetStmt();
    ParseResult<TryCatchStmt*>  parseTryCatchStmt();
    ParseResult<SelectStmt*>    parseSelectStmt();
    ParseResult<UnsafeStmt*>    parseUnsafeBlock();
    ParseResult<ExprStmt*>      parseExprStmt();

    // ── Pattern parsing (Chapter 17, match patterns) ─────────────────────────

    ParseResult<PatternNode*>   parsePattern();
    ParseResult<PatternNode*>   parsePatternAtom();
    // OR patterns: 1 | 2 | 3
    ParseResult<PatternNode*>   parseOrPattern();

    // ── Expression parsing — Pratt parser ────────────────────────────────────

    // Main entry. minPrec controls which operators are consumed.
    ParseResult<ExprNode*>      parseExpr(Prec minPrec = Prec::Assignment);

    // Parse a primary expression (literal, identifier, grouped expr, etc.)
    ParseResult<ExprNode*>      parsePrimary();

    // Parse postfix operations: () [] . ?. ++ -- ? after an expression
    ParseResult<ExprNode*>      parsePostfix(ExprNode* base);

    // Parse unary prefix operators: ! ~ - * & ++ -- move
    ParseResult<ExprNode*>      parseUnary();

    // Parse infix binary expression given left side and minimum precedence
    ParseResult<ExprNode*>      parseInfix(ExprNode* left, Prec minPrec);

    // ── Specific expression parsers ───────────────────────────────────────────

    ParseResult<ExprNode*>      parseLiteral();
    ParseResult<ExprNode*>      parseIdentifierExpr();
    ParseResult<ExprNode*>      parseGroupedExprOrTuple();
    ParseResult<ExprNode*>      parseListLiteral();
    ParseResult<ExprNode*>      parseMapOrSetLiteral();
    ParseResult<ExprNode*>      parseLambda();
    ParseResult<ExprNode*>      parseIfExpr();
    ParseResult<ExprNode*>      parseMatchExpr();
    ParseResult<ExprNode*>      parseStructLiteral(ExprNode* typeName);
    ParseResult<ExprNode*>      parseCallExpr(ExprNode* callee);
    ParseResult<ExprNode*>      parseIndexExpr(ExprNode* base);
    ParseResult<ExprNode*>      parseMemberExpr(ExprNode* base);
    ParseResult<ExprNode*>      parseModulePathExpr(ExprNode* base);

    // ── Type parsing ──────────────────────────────────────────────────────────

    ParseResult<TypeNode*>      parseType();
    ParseResult<TypeNode*>      parseTypeAtom();
    ParseResult<TypeNode*>      parseFnType();
    ParseResult<TypeNode*>      parseGenericType(TypeNode* base);

    // ── Argument list helpers ─────────────────────────────────────────────────

    struct ParsedArg {
        std::string  name;     // empty if positional
        ExprNode*    value;
        SourceLocation loc;
    };
    std::vector<ParsedArg> parseArgList();

    // ── f-string segment parsing ──────────────────────────────────────────────

    ParseResult<FmtStringExpr*> parseFmtString(Token fmtTok);

    // ── Match arm list ────────────────────────────────────────────────────────

    std::vector<MatchArm*> parseMatchArms();
    ParseResult<MatchArm*> parseMatchArm();

    // ── Utility ───────────────────────────────────────────────────────────────

    // Emit "expected X, found Y" diagnostic
    void expectedError(std::string_view what);

    // True if we are currently inside an import statement context
    // Used to determine if :: is valid (only valid inside import)
    bool inImportContext_ = false;

    // True if we are currently inside an async fn context
    bool inAsyncContext_  = false;

    // True if we are currently inside a fn* generator context
    bool inGeneratorContext_ = false;

    // Loop depth (for skip/stop validation)
    uint32_t loopDepth_ = 0;
};

} // namespace vex