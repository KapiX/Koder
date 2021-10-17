/*
 * Copyright 2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include <gtest/gtest.h>

#include "support/Utils.h"

// GetFileName

TEST(GetFileNameTest, HandlesNoExtension) {
	const std::string result = GetFileName("test");
	ASSERT_EQ(result, "test");
}

TEST(GetFileNameTest, HandlesDotFile) {
	const std::string result = GetFileName(".test");
	ASSERT_EQ(result, ".test");
}

TEST(GetFileNameTest, HandlesDotFileWithExtension) {
	const std::string result = GetFileName(".test.second");
	ASSERT_EQ(result, ".test");
}

TEST(GetFileNameTest, HandlesDotFileWithEmptyExtension) {
	const std::string result = GetFileName(".test.");
	ASSERT_EQ(result, ".test");
}

TEST(GetFileNameTest, HandlesFileWithEmptyExtension) {
	const std::string result = GetFileName("test.");
	ASSERT_EQ(result, "test");
}

TEST(GetFileNameTest, HandlesFileWithExtension) {
	const std::string result = GetFileName("test.txt");
	ASSERT_EQ(result, "test");
}

TEST(GetFileNameTest, HandlesFileWithLongerExtension) {
	const std::string result = GetFileName("testing.extension");
	ASSERT_EQ(result, "testing");
}

TEST(GetFileNameTest, HandlesFileWithTwoExtensions) {
	const std::string result = GetFileName("testing.long.extension");
	ASSERT_EQ(result, "testing.long");
}

// GetFileExtension

TEST(GetFileExtensionTest, HandlesNoExtension) {
	const std::string result = GetFileExtension("test");
	ASSERT_EQ(result, "");
}

TEST(GetFileExtensionTest, HandlesDotFile) {
	const std::string result = GetFileExtension(".test");
	ASSERT_EQ(result, "");
}

TEST(GetFileExtensionTest, HandlesDotFileWithExtension) {
	const std::string result = GetFileExtension(".test.second");
	ASSERT_EQ(result, "second");
}

TEST(GetFileExtensionTest, HandlesDotFileWithEmptyExtension) {
	const std::string result = GetFileExtension(".test.");
	ASSERT_EQ(result, "");
}

TEST(GetFileExtensionTest, HandlesFileWithEmptyExtension) {
	const std::string result = GetFileExtension("test.");
	ASSERT_EQ(result, "");
}

TEST(GetFileExtensionTest, HandlesFileWithExtension) {
	const std::string result = GetFileExtension("test.txt");
	ASSERT_EQ(result, "txt");
}

TEST(GetFileExtensionTest, HandlesFileWithLongerExtension) {
	const std::string result = GetFileExtension("testing.extension");
	ASSERT_EQ(result, "extension");
}

TEST(GetFileExtensionTest, HandlesFileWithTwoExtensions) {
	const std::string result = GetFileExtension("testing.long.extension");
	ASSERT_EQ(result, "extension");
}

// ParseFileArgument

TEST(ParseFileArgumentTest, HandlesFileNameAndNoReturnParameters) {
	const std::string result = ParseFileArgument("test.txt");
	EXPECT_EQ(result, "test.txt");
}

TEST(ParseFileArgumentTest, HandlesFileNameNoLineAndNoReturnParameters) {
	const std::string result = ParseFileArgument("test.txt:");
	EXPECT_EQ(result, "test.txt");
}

TEST(ParseFileArgumentTest, HandlesFileNameLineAndNoReturnParameters) {
	const std::string result = ParseFileArgument("test.txt:2");
	EXPECT_EQ(result, "test.txt");
}

TEST(ParseFileArgumentTest, HandlesFileNameLineNoColumnAndNoReturnParameters) {
	const std::string result = ParseFileArgument("test.txt:2:");
	EXPECT_EQ(result, "test.txt");
}

TEST(ParseFileArgumentTest, HandlesFileNameNoLine) {
	int32 line = 0;
	int32 column = 0;
	const std::string result = ParseFileArgument("test.txt:", &line, &column);
	// line is undefined
	// column is undefined
	EXPECT_EQ(result, "test.txt");
}

TEST(ParseFileArgumentTest, HandlesFileNameLine) {
	int32 line = 0;
	int32 column = 0;
	const std::string result = ParseFileArgument("test.txt:2", &line, &column);
	EXPECT_EQ(line, 2);
	// column is undefined
	EXPECT_EQ(result, "test.txt");
}

TEST(ParseFileArgumentTest, HandlesFileNameLineNoColumn) {
	int32 line = 0;
	int32 column = 0;
	const std::string result = ParseFileArgument("test.txt:2:", &line, &column);
	EXPECT_EQ(line, 2);
	// column is undefined
	EXPECT_EQ(result, "test.txt");
}

TEST(ParseFileArgumentTest, HandlesFileNameNoLineNoColumnAndNoReturnParameters) {
	const std::string result = ParseFileArgument("test.txt::");
	EXPECT_EQ(result, "test.txt");
}

TEST(ParseFileArgumentTest, HandlesFileNameLineColumnAndOneReturnParameter) {
	int32 line = 0;
	const std::string result = ParseFileArgument("test.txt:2:5", &line);
	EXPECT_EQ(line, 2);
	EXPECT_EQ(result, "test.txt");
}

TEST(ParseFileArgumentTest, HandlesFileNameNoLineColumn) {
	int32 line = 0;
	int32 column = 0;
	const std::string result = ParseFileArgument("test.txt::32", &line, &column);
	// line is undefined
	EXPECT_EQ(column, 32);
	EXPECT_EQ(result, "test.txt");
}

TEST(ParseFileArgumentTest, HandlesFileNameNoLineColumnAndOneReturnParameter) {
	int32 column = 0;
	const std::string result = ParseFileArgument("test.txt::32", nullptr, &column);
	EXPECT_EQ(column, 32);
	EXPECT_EQ(result, "test.txt");
}

TEST(ParseFileArgumentTest, HandlesFileNameLineColumn) {
	int32 line = 0;
	int32 column = 0;
	const std::string result = ParseFileArgument("test.txt:2:5", &line, &column);
	EXPECT_EQ(line, 2);
	EXPECT_EQ(column, 5);
	EXPECT_EQ(result, "test.txt");
}

TEST(ParseFileArgumentTest, HandlesLargeLineAndColumnValues) {
	int32 line = 0;
	int32 column = 0;
	const std::string result = ParseFileArgument("test.txt:2332:512354", &line, &column);
	EXPECT_EQ(line, 2332);
	EXPECT_EQ(column, 512354);
	EXPECT_EQ(result, "test.txt");
}

TEST(ParseFileArgumentTest, HandlesNegativeLineAndColumnValues) {
	int32 line = 0;
	int32 column = 0;
	const std::string result = ParseFileArgument("test.txt:-23:-120", &line, &column);
	EXPECT_EQ(line, -23);
	EXPECT_EQ(column, -120);
	EXPECT_EQ(result, "test.txt");
}

TEST(ParseFileArgumentTest, OnlyFileNameReturnNegativeOneForLineAndColumn) {
	int32 line = 0;
	int32 column = 0;
	const std::string result = ParseFileArgument("test.txt", &line, &column);
	EXPECT_EQ(line, -1);
	EXPECT_EQ(column, -1);
	EXPECT_EQ(result, "test.txt");
}

TEST(ParseFileArgumentTest, NoLineAndNoColumnReturnNegativeOne) {
	int32 line = 0;
	int32 column = 0;
	const std::string result = ParseFileArgument("test.txt::", &line, &column);
	EXPECT_EQ(line, -1);
	EXPECT_EQ(column, -1);
	EXPECT_EQ(result, "test.txt");
}

TEST(ParseFileArgumentTest, IgnoresIncompleteNegativeNumbers) {
	int32 line = 0;
	int32 column = 0;
	const std::string result = ParseFileArgument("test.txt:-:-", &line, &column);
	EXPECT_EQ(result, "test.txt:-:-");
}

TEST(ParseFileArgumentTest, Issue137IgnoresURLs) {
	int32 line = 0;
	int32 column = 0;
	const std::string result = ParseFileArgument("https://datatracker.ietf.org/drafts/current/", &line, &column);
	EXPECT_EQ(result, "https://datatracker.ietf.org/drafts/current/");
}
