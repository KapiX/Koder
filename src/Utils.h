/*
 * Copyright 2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef UTILS_H
#define UTILS_H


#include <string>

#include <MessageFilter.h>


class BBitmap;


std::string GetFileName(const std::string filename);
std::string GetFileExtension(const std::string filename);
// Gets an icon from executable's resources
void GetVectorIcon(const std::string icon, BBitmap* bitmap);


class KeyDownMessageFilter : public BMessageFilter
{
public:
							KeyDownMessageFilter(char key, uint32 commandToSend);

	virtual	filter_result	Filter(BMessage* message, BHandler** target);

private:
			char			fKey;
			uint32			fCommandToSend;
};


#endif // UTILS_H
