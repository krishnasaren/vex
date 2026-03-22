// tests/unit/Core/DiagnosticTest.cpp
// tests/unit/Core/DiagnosticTest.cpp
#include "vex/Core/DiagnosticEngine.h"
#include "vex/Core/DiagnosticConsumer.h"
#include "vex/Core/SourceManager.h"
#include <gtest/gtest.h>

using namespace vex;

class DiagnosticTest : public ::testing::Test {
protected:
    SourceManager srcMgr;
    StoringDiagnosticConsumer consumer;
    std::unique_ptr<DiagnosticEngine> engine;

    void SetUp() override {
        engine = std::make_unique<DiagnosticEngine>(consumer);
    }
};

TEST_F(DiagnosticTest, EmitError) {
    engine->emitSimple(DiagID::LEX_UnknownChar,
                       SourceLocation::invalid(),
                       "unexpected character '@'");
    EXPECT_EQ(engine->errorCount(), 1u);
    EXPECT_TRUE(engine->hasErrors());
    ASSERT_EQ(consumer.diagnostics().size(), 1u);
    EXPECT_EQ(consumer.diagnostics()[0].id(), DiagID::LEX_UnknownChar);
}

TEST_F(DiagnosticTest, EmitWarning) {
    engine->emitSimple(DiagID::SEMA_ShadowWarning,
                       SourceLocation::invalid(),
                       "variable shadows outer declaration");
    EXPECT_EQ(engine->warningCount(), 1u);
    EXPECT_FALSE(engine->hasErrors());
}

TEST_F(DiagnosticTest, WarningsAsErrors) {
    engine->setWarningsAsErrors(true);
    engine->emitSimple(DiagID::SEMA_ShadowWarning,
                       SourceLocation::invalid(),
                       "shadow warning");
    EXPECT_TRUE(engine->hasErrors());
}

TEST_F(DiagnosticTest, ErrorLimit) {
    engine->setErrorLimit(3);
    for (int i = 0; i < 3; i++) {
        engine->emitSimple(DiagID::LEX_UnknownChar,
                           SourceLocation::invalid(), "err");
    }
    EXPECT_TRUE(engine->shouldAbort());
}

TEST_F(DiagnosticTest, DiagnosticBuilder) {
    engine->report(DiagID::TYPE_Mismatch,
                   SourceLocation::invalid(),
                   "type mismatch: expected int, found str")
          .addNote("declared as int here")
          .setHint("consider adding a cast: int(x)");

    ASSERT_EQ(consumer.diagnostics().size(), 1u);
    auto& d = consumer.diagnostics()[0];
    EXPECT_EQ(d.id(), DiagID::TYPE_Mismatch);
    EXPECT_FALSE(d.notes().empty());
    EXPECT_FALSE(d.hint().empty());
}

TEST_F(DiagnosticTest, MultipleErrors) {
    for (auto id : {DiagID::LEX_UnknownChar,
                    DiagID::PARSE_Expected,
                    DiagID::SEMA_UndefinedName}) {
        engine->emitSimple(id, SourceLocation::invalid(), "error");
    }
    EXPECT_EQ(engine->errorCount(), 3u);
    EXPECT_EQ(consumer.diagnostics().size(), 3u);
}

TEST_F(DiagnosticTest, SuppressWarnings) {
    engine->setSuppressWarnings(true);
    engine->emitSimple(DiagID::SEMA_ShadowWarning,
                       SourceLocation::invalid(), "warning");
    EXPECT_EQ(engine->warningCount(), 0u);
    EXPECT_TRUE(consumer.diagnostics().empty());
}