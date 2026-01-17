/*
 * Copyright 2018 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef FILE_H
#define FILE_H


#include <File.h>
#include <Message.h>

#include <string>
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

	int32				ReadCaretPosition();
	void				WriteCaretPosition(int32 caretPos);
	std::string			ReadMimeType();
	void				WriteMimeType(std::string mimeType);
	BMessage			ReadBookmarks();
	void				WriteBookmarks(BMessage bookmarks);

	status_t			Monitor(bool enable, BHandler* handler);

	static	status_t	Monitor(BStatable* file, bool enable,
							BHandler* handler);
	static	bool		CanWrite(BStatable* file);
	static	status_t	SetWritable(BStatable* file, bool writable);
};


/**
 * BackupFileGuard will create a backup of a file and let it exist until the
 * guard goes out of scope. If SaveSuccessful() checkpoint is not reached
 * (for example the program crashed or an exception was thrown) the backup file
 * will not be removed.
 * If path does not exist or is empty does nothing.
 */
class BackupFileGuard {
public:
	BackupFileGuard(const char* path, BHandler *handler = nullptr);
	~BackupFileGuard();

	void		SaveSuccessful();
private:
	std::string	fPath;
	bool		fSuccess;
};


#endif // FILE_H
