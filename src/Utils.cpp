/*
 * Copyright 2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Utils.h"

#include <Application.h>
#include <Bitmap.h>
#include <IconUtils.h>
#include <Resources.h>


std::string
GetFileName(const std::string filename)
{
	size_t pos = filename.rfind('.');
	if(pos != std::string::npos)
		return filename.substr(0, pos);
	return filename;
}


std::string
GetFileExtension(const std::string filename)
{
	size_t pos = filename.rfind('.');
	if(pos != std::string::npos)
		return filename.substr(pos + 1);
	return "";
}


void
GetVectorIcon(const std::string icon, BBitmap* bitmap)
{
	if(bitmap == nullptr)
		return;

	BResources* resources = BApplication::AppResources();
	size_t size;
	const uint8* rawIcon;
	rawIcon = (const uint8*) resources->LoadResource(B_VECTOR_ICON_TYPE, icon.c_str(), &size);
	if(rawIcon == nullptr)
		return;

	BIconUtils::GetVectorIcon(rawIcon, size, bitmap);
}
