// tests/unit/Lexer/LexerTest.cpp
// tests/unit/Lexer/LexerTest.cpp
#include "vex/Lexer/Lexer.h"
#include "vex/Core/SourceManager.h"
#include "vex/Core/DiagnosticConsumer.h"
#include "vex/Core/DiagnosticEngine.h"
#include "vex/Core/MemoryBuffer.h"
#include <gtest/gtest.h>
#include <vector>

using namespace vex;

class LexerTest : public ::testing::Test {
protected:
    SourceManager srcMgr;
    StoringDiagnosticConsumer consumer;
    std::unique_ptr<DiagnosticEngine> diags;

    void SetUp() override {
        diags = std::make_unique<DiagnosticEngine>(consumer);
    }

    std::vector<Token> lex(std::string_view src) {
        auto buf = MemoryBuffer::fromString(src, "test.vex");
        FileID id = srcMgr.addBuffer(std::move(buf), "test.vex");
        const MemoryBuffer* mb = srcMgr.getBuffer(id);
        Lexer lexer(*mb, id, srcMgr, *diags);
        return lexer.lexAll();
    }

    void expectKinds(std::string_view src, std::vector<TokenKind> expected) {
        auto tokens = lex(src);
        expected.push_back(TokenKind::Eof);
        ASSERT_EQ(tokens.size(), expected.size())
            << "Token count mismatch for input: " << src;
        for (size_t i = 0; i < expected.size(); i++) {
            EXPECT_EQ(tokens[i].kind(), expected[i])
                << "Token " << i << " mismatch for: " << src
                << " (expected " << (int)expected[i]
                << " got " << (int)tokens[i].kind() << ")";
        }
    }
};

TEST_F(LexerTest, EmptySource) {
    auto tokens = lex("");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_TRUE(tokens[0].isEof());
}

TEST_F(LexerTest, Keywords) {
    expectKinds("fn struct class enum",
        {TokenKind::KW_fn, TokenKind::KW_struct,
         TokenKind::KW_class, TokenKind::KW_enum});
}

TEST_F(LexerTest, Identifiers) {
    auto tokens = lex("hello world _private CamelCase");
    EXPECT_EQ(tokens[0].kind(), TokenKind::Identifier);
    EXPECT_EQ(tokens[0].text(), "hello");
    EXPECT_EQ(tokens[1].kind(), TokenKind::Identifier);
    EXPECT_EQ(tokens[3].kind(), TokenKind::Identifier);
    EXPECT_EQ(tokens[3].text(), "CamelCase");
}

TEST_F(LexerTest, IntegerLiterals) {
    auto tokens = lex("42 0xFF 0b1010 0o755 1_000_000");
    EXPECT_EQ(tokens[0].kind(), TokenKind::IntLiteral);
    EXPECT_EQ(tokens[0].text(), "42");
    EXPECT_EQ(tokens[1].kind(), TokenKind::IntLiteral);
    EXPECT_EQ(tokens[1].intBase(), 16);
    EXPECT_EQ(tokens[2].intBase(), 2);
    EXPECT_EQ(tokens[3].intBase(), 8);
}

TEST_F(LexerTest, FloatLiterals) {
    auto tokens = lex("3.14 1.5e10 2.7f");
    EXPECT_EQ(tokens[0].kind(), TokenKind::FloatLiteral);
    EXPECT_EQ(tokens[1].kind(), TokenKind::FloatLiteral);
    EXPECT_EQ(tokens[2].floatSuffix(), FloatSuffix::f);
}

TEST_F(LexerTest, StringLiterals) {
    auto tokens = lex(R"("hello" r"\n raw" `multiline`)");
    EXPECT_EQ(tokens[0].kind(), TokenKind::StringLiteral);
    EXPECT_EQ(tokens[1].kind(), TokenKind::RawString);
    EXPECT_EQ(tokens[2].kind(), TokenKind::MultilineStr);
}

TEST_F(LexerTest, BoolAndNull) {
    expectKinds("true false null",
        {TokenKind::BoolTrue, TokenKind::BoolFalse, TokenKind::Null});
}

TEST_F(LexerTest, Operators) {
    expectKinds("+ - * / % **",
        {TokenKind::Plus, TokenKind::Minus, TokenKind::Star,
         TokenKind::Slash, TokenKind::Percent, TokenKind::StarStar});
}

TEST_F(LexerTest, CompoundOperators) {
    expectKinds("+= -= *= /= %= **= := ::",
        {TokenKind::PlusAssign, TokenKind::MinusAssign,
         TokenKind::StarAssign, TokenKind::SlashAssign,
         TokenKind::PercentAssign, TokenKind::StarStarAssign,
         TokenKind::ColonAssign, TokenKind::ColonColon});
}

TEST_F(LexerTest, WrappingAndSaturatingOps) {
    expectKinds("+% -% *% +| -| *|",
        {TokenKind::WrapAdd, TokenKind::WrapSub, TokenKind::WrapMul,
         TokenKind::SatAdd,  TokenKind::SatSub,  TokenKind::SatMul});
}

TEST_F(LexerTest, RangeOperators) {
    expectKinds(".. ..= ...",
        {TokenKind::DotDot, TokenKind::DotDotEq, TokenKind::DotDotDot});
}

TEST_F(LexerTest, NullableOperators) {
    expectKinds("?. ?? ?",
        {TokenKind::QuestionDot, TokenKind::QuestionQuestion, TokenKind::Question});
}

TEST_F(LexerTest, LineComment) {
    auto tokens = lex("x // this is a comment\ny");
    EXPECT_EQ(tokens[0].kind(), TokenKind::Identifier);
    EXPECT_EQ(tokens[0].text(), "x");
    EXPECT_EQ(tokens[1].kind(), TokenKind::Identifier);
    EXPECT_EQ(tokens[1].text(), "y");
}

TEST_F(LexerTest, BlockComment) {
    auto tokens = lex("x /* block */ y");
    EXPECT_EQ(tokens[0].text(), "x");
    EXPECT_EQ(tokens[1].text(), "y");
}

TEST_F(LexerTest, NestedBlockComment) {
    auto tokens = lex("a /* outer /* inner */ still */ b");
    EXPECT_EQ(tokens[0].text(), "a");
    EXPECT_EQ(tokens[1].text(), "b");
}

TEST_F(LexerTest, IntegerSuffixes) {
    auto tokens = lex("42u 255u8 1000u64 -1i8 42L");
    EXPECT_EQ(tokens[0].intSuffix(), IntSuffix::u);
    EXPECT_EQ(tokens[1].intSuffix(), IntSuffix::u8);
    EXPECT_EQ(tokens[2].intSuffix(), IntSuffix::u64);
}

TEST_F(LexerTest, DurationLiterals) {
    auto tokens = lex("1000ms 5s 500us 100ns");
    EXPECT_EQ(tokens[0].kind(), TokenKind::IntLiteral);
    EXPECT_EQ(tokens[0].intSuffix(), IntSuffix::ms);
    EXPECT_EQ(tokens[1].intSuffix(), IntSuffix::s);
    EXPECT_EQ(tokens[2].intSuffix(), IntSuffix::us);
    EXPECT_EQ(tokens[3].intSuffix(), IntSuffix::ns);
}

TEST_F(LexerTest, UnterminatedString) {
    lex("\"unterminated");
    EXPECT_GT(consumer.errorCount(), 0u);
}

TEST_F(LexerTest, AllBrackets) {
    expectKinds("( ) [ ] { }",
        {TokenKind::LParen, TokenKind::RParen,
         TokenKind::LBracket, TokenKind::RBracket,
         TokenKind::LBrace, TokenKind::RBrace});
}

TEST_F(LexerTest, SourceLocation) {
    auto tokens = lex("fn main");
    EXPECT_EQ(tokens[0].location().line(), 1u);
    EXPECT_EQ(tokens[0].location().col(), 1u);
    EXPECT_EQ(tokens[1].location().line(), 1u);
    EXPECT_EQ(tokens[1].location().col(), 4u);
}

TEST_F(LexerTest, MultiLineSourceLocation) {
    auto tokens = lex("fn\nmain");
    EXPECT_EQ(tokens[0].location().line(), 1u);
    EXPECT_EQ(tokens[1].location().line(), 2u);
}

TEST_F(LexerTest, ModKeyword) {
    expectKinds("mod", {TokenKind::KW_mod});
}

TEST_F(LexerTest, PipeOperators) {
    expectKinds("|> | ||",
        {TokenKind::PipeArrow, TokenKind::Pipe, TokenKind::PipePipe});
}

TEST_F(LexerTest, ArrowOperators) {
    expectKinds("-> =>",
        {TokenKind::Arrow, TokenKind::FatArrow});
}