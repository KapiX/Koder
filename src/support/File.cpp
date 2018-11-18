/*
 * Copyright 2018 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "File.h"


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
