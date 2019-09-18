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
