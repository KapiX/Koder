/*
 * Copyright 2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef UTILS_H
#define UTILS_H


#include <string>


class BBitmap;


std::string GetFileName(const std::string filename);
std::string GetFileExtension(const std::string filename);
// Gets an icon from executable's resources
void GetVectorIcon(const std::string icon, BBitmap* bitmap);


#endif // UTILS_H
