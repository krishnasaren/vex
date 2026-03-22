// tests/unit/Core/SourceLocationTest.cpp
// tests/unit/Core/SourceLocationTest.cpp
#include "vex/Core/SourceManager.h"
#include "vex/Core/MemoryBuffer.h"
#include <gtest/gtest.h>

using namespace vex;

class SourceManagerTest : public ::testing::Test {
protected:
    SourceManager srcMgr;
};

TEST_F(SourceManagerTest, AddBufferAndRetrieve) {
    auto buf = MemoryBuffer::fromString("hello world", "test.vex");
    FileID id = srcMgr.addBuffer(std::move(buf), "test.vex");

    EXPECT_TRUE(id.isValid());
    EXPECT_EQ(srcMgr.getPath(id), "test.vex");

    const MemoryBuffer* mb = srcMgr.getBuffer(id);
    ASSERT_NE(mb, nullptr);
    EXPECT_EQ(mb->size(), 11u);
    EXPECT_EQ(mb->content(), "hello world");
}

TEST_F(SourceManagerTest, GetLocationFromPointer) {
    auto buf = MemoryBuffer::fromString("line1\nline2\nline3", "test.vex");
    const char* data = buf->data();
    FileID id = srcMgr.addBuffer(std::move(buf), "test.vex");

    // Start of file = line 1, col 1
    SourceLocation loc1 = srcMgr.getLocation(id, data);
    EXPECT_EQ(loc1.line(), 1u);
    EXPECT_EQ(loc1.col(),  1u);

    // After "line1\n" = line 2, col 1 (byte offset 6)
    SourceLocation loc2 = srcMgr.getLocation(id, data + 6);
    EXPECT_EQ(loc2.line(), 2u);
    EXPECT_EQ(loc2.col(),  1u);

    // Middle of line 2: "line2" col 3 = byte 6+2 = 8
    SourceLocation loc3 = srcMgr.getLocation(id, data + 8);
    EXPECT_EQ(loc3.line(), 2u);
    EXPECT_EQ(loc3.col(),  3u);
}

TEST_F(SourceManagerTest, GetLineText) {
    auto buf = MemoryBuffer::fromString("line1\nline2\nline3", "test.vex");
    FileID id = srcMgr.addBuffer(std::move(buf), "test.vex");

    SourceLocation loc1(id, 1, 1);
    EXPECT_EQ(srcMgr.getLineText(loc1), "line1");

    SourceLocation loc2(id, 2, 1);
    EXPECT_EQ(srcMgr.getLineText(loc2), "line2");

    SourceLocation loc3(id, 3, 1);
    EXPECT_EQ(srcMgr.getLineText(loc3), "line3");
}

TEST_F(SourceManagerTest, InvalidFileID) {
    FileID invalid = FileID::invalid();
    EXPECT_FALSE(invalid.isValid());
    EXPECT_EQ(srcMgr.getBuffer(invalid), nullptr);
    EXPECT_TRUE(srcMgr.getPath(invalid).empty());
}

TEST_F(SourceManagerTest, MultipleFiles) {
    auto b1 = MemoryBuffer::fromString("source1", "a.vex");
    auto b2 = MemoryBuffer::fromString("source2", "b.vex");

    FileID id1 = srcMgr.addBuffer(std::move(b1), "a.vex");
    FileID id2 = srcMgr.addBuffer(std::move(b2), "b.vex");

    EXPECT_NE(id1.id, id2.id);
    EXPECT_EQ(srcMgr.getPath(id1), "a.vex");
    EXPECT_EQ(srcMgr.getPath(id2), "b.vex");
    EXPECT_EQ(srcMgr.fileCount(), 2u);
}