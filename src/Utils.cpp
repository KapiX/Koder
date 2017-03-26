/*
 * Copyright 2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Utils.h"


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
