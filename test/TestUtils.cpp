/*
 * Copyright 2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include <gtest/gtest.h>

#include "support/Utils.h"

// GetFileName

TEST(GetFileNameTest, HandlesNoExtension) {
	const std::string name = "test";
	const std::string result = GetFileName(name);
	const std::string expected = "test";
	ASSERT_EQ(result, expected);
}

TEST(GetFileNameTest, HandlesDotFile) {
	// FIXME: that's wrong
	const std::string name = ".test";
	const std::string result = GetFileName(name);
	const std::string expected = ".test";
	ASSERT_EQ(result, expected);
}

TEST(GetFileNameTest, HandlesDotFileWithExtension) {
	const std::string name = ".test.second";
	const std::string result = GetFileName(name);
	const std::string expected = ".test";
	ASSERT_EQ(result, expected);
}

TEST(GetFileNameTest, HandlesDotFileWithEmptyExtension) {
	const std::string name = ".test.";
	const std::string result = GetFileName(name);
	const std::string expected = ".test";
	ASSERT_EQ(result, expected);
}

TEST(GetFileNameTest, HandlesFileWithEmptyExtension) {
	const std::string name = "test.";
	const std::string result = GetFileName(name);
	const std::string expected = "test";
	ASSERT_EQ(result, expected);
}

TEST(GetFileNameTest, HandlesFileWithExtension) {
	const std::string name = "test.txt";
	const std::string result = GetFileName(name);
	const std::string expected = "test";
	ASSERT_EQ(result, expected);
}

TEST(GetFileNameTest, HandlesFileWithLongerExtension) {
	const std::string name = "testing.extension";
	const std::string result = GetFileName(name);
	const std::string expected = "testing";
	ASSERT_EQ(result, expected);
}

TEST(GetFileNameTest, HandlesFileWithTwoExtensions) {
	const std::string name = "testing.long.extension";
	const std::string result = GetFileName(name);
	const std::string expected = "testing.long";
	ASSERT_EQ(result, expected);
}

// GetFileExtension

TEST(GetFileExtensionTest, HandlesNoExtension) {
	const std::string name = "test";
	const std::string result = GetFileExtension(name);
	const std::string expected = "";
	ASSERT_EQ(result, expected);
}

TEST(GetFileExtensionTest, HandlesDotFile) {
	const std::string name = ".test";
	const std::string result = GetFileExtension(name);
	const std::string expected = "";
	ASSERT_EQ(result, expected);
}

TEST(GetFileExtensionTest, HandlesDotFileWithExtension) {
	const std::string name = ".test.second";
	const std::string result = GetFileExtension(name);
	const std::string expected = "second";
	ASSERT_EQ(result, expected);
}

TEST(GetFileExtensionTest, HandlesDotFileWithEmptyExtension) {
	const std::string name = ".test.";
	const std::string result = GetFileExtension(name);
	const std::string expected = "";
	ASSERT_EQ(result, expected);
}

TEST(GetFileExtensionTest, HandlesFileWithEmptyExtension) {
	const std::string name = "test.";
	const std::string result = GetFileExtension(name);
	const std::string expected = "";
	ASSERT_EQ(result, expected);
}

TEST(GetFileExtensionTest, HandlesFileWithExtension) {
	const std::string name = "test.txt";
	const std::string result = GetFileExtension(name);
	const std::string expected = "txt";
	ASSERT_EQ(result, expected);
}

TEST(GetFileExtensionTest, HandlesFileWithLongerExtension) {
	const std::string name = "testing.extension";
	const std::string result = GetFileExtension(name);
	const std::string expected = "extension";
	ASSERT_EQ(result, expected);
}

TEST(GetFileExtensionTest, HandlesFileWithTwoExtensions) {
	const std::string name = "testing.long.extension";
	const std::string result = GetFileExtension(name);
	const std::string expected = "extension";
	ASSERT_EQ(result, expected);
}
