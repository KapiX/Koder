/*
 * Copyright 2017-2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef UTILS_H
#define UTILS_H


#include <string>
#include <type_traits>

#include <Alert.h>
#include <MessageFilter.h>


class BBitmap;
class BCheckBox;
class BRadioButton;


std::string GetFileName(const std::string filename);
std::string GetFileExtension(const std::string filename);
// Gets an icon from executable's resources
void GetVectorIcon(const std::string icon, BBitmap* bitmap);


std::string ParseFileArgument(const std::string argument,
	int32* line = nullptr, int32* column = nullptr);


template<typename T>
bool IsChecked(T* control);
template<typename T>
void SetChecked(T* control, bool checked = true);


void OKAlert(const char* title, const char* message,
	alert_type type = B_INFO_ALERT);


int32 rgb_colorToSciColor(rgb_color color);


class KeyDownMessageFilter : public BMessageFilter
{
public:
							KeyDownMessageFilter(uint32 commandToSend,
								char key, uint32 modifiers = 0);

	virtual	filter_result	Filter(BMessage* message, BHandler** target);

private:
			char			fKey;
			uint32			fModifiers;
			uint32			fCommandToSend;
};


#endif // UTILS_H
