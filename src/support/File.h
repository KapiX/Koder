/*
 * Copyright 2018 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef FILE_H
#define FILE_H


#include <File.h>

#include <vector>


struct entry_ref;
class BEntry;


class File : public BFile {
public:
	File(const entry_ref* ref, uint32 openMode);
	File(const BEntry* entry, uint32 openMode);
	File(const char* path, uint32 openMode);

	std::vector<char>	Read();
	void				Write(std::vector<char> &buffer);
private:
};


#endif // FILE_H
