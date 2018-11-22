/*
 * Copyright 2018 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "File.h"

#include <NodeInfo.h>
#include <NodeMonitor.h>
#include <kernel/fs_attr.h>

#include <vector>
#include <string>


namespace {

const std::string kCaretPositionAttribute = "be:caret_position";
const std::string kBookmarksAttribute = "koder:bookmarks";

}


File::File(const entry_ref* ref, uint32 openMode)
	:
	BFile(ref, openMode)
{
}


File::File(const BEntry* entry, uint32 openMode)
	:
	BFile(entry, openMode)
{
}


File::File(const char* path, uint32 openMode)
	:
	BFile(path, openMode)
{
}


std::vector<char>
File::Read()
{
	off_t size;
	GetSize(&size);
	std::vector<char> buffer(size + 1);
	BFile::Read(buffer.data(), buffer.size());
	buffer[size] = 0;
	return buffer;
}


void
File::Write(std::vector<char> &buffer)
{
	BFile::Write(buffer.data(), buffer.size() - 1);
}


void
File::WriteCaretPosition(int32 caretPos)
{
	BFile::WriteAttr(kCaretPositionAttribute.c_str(), B_INT32_TYPE, 0,
		&caretPos, sizeof(int32));
}


int32
File::ReadCaretPosition()
{
	int32 caretPos = 0;
	BFile::ReadAttr(kCaretPositionAttribute.c_str(), B_INT32_TYPE, 0,
		&caretPos, sizeof(int32));
	return caretPos;
}


void
File::WriteMimeType(std::string mimeType)
{
	BNodeInfo info(this);
	info.SetType(mimeType.c_str());
}


std::string
File::ReadMimeType()
{
	std::string mimeType(B_MIME_TYPE_LENGTH, 0);
	BNodeInfo info(this);
	info.GetType(mimeType.data());
	return mimeType;
}


void
File::WriteBookmarks(BMessage bookmarks)
{
	BMallocIO mallocIO;
	bookmarks.Flatten(&mallocIO);
	BFile::WriteAttr(kBookmarksAttribute.c_str(), B_MESSAGE_TYPE, 0,
		mallocIO.Buffer(), mallocIO.BufferLength());
}


BMessage
File::ReadBookmarks()
{
	BMessage bookmarks;
	attr_info info;
	BFile::GetAttrInfo(kBookmarksAttribute.c_str(), &info);
	if(info.type == B_MESSAGE_TYPE) {
		std::vector<char> buffer(info.size);
		BFile::ReadAttr(kBookmarksAttribute.c_str(), B_MESSAGE_TYPE, 0,
			buffer.data(), buffer.size());
		BMemoryIO memIO(buffer.data(), buffer.size());
		bookmarks.Unflatten(&memIO);
	}
	return bookmarks;
}


status_t
File::Monitor(bool enable, BHandler* handler)
{
	return File::Monitor(this, enable, handler);
}


status_t
File::Monitor(BStatable* file, bool enable, BHandler* handler)
{
	if(file == nullptr)
		return B_BAD_VALUE;

	uint32 flags = (enable ? B_WATCH_NAME | B_WATCH_STAT : B_STOP_WATCHING);
	node_ref nref;
	file->GetNodeRef(&nref);
	return watch_node(&nref, flags, handler);
}

bool
File::CanWrite(BStatable* file)
{
	if(file == nullptr)
		return false;

	mode_t permissions;
	if(file->GetPermissions(&permissions) < B_OK) {
		return false;
	}

	if(permissions & (S_IWUSR | S_IWGRP | S_IWOTH)) {
		return true;
	}
	return false;
}
