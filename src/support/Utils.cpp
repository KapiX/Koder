/*
 * Copyright 2017-2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Utils.h"

#include <Alert.h>
#include <Application.h>
#include <Bitmap.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <IconUtils.h>
#include <MessageFilter.h>
#include <RadioButton.h>
#include <Resources.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Utilities"


std::string
GetFileName(const std::string filename)
{
	size_t pos = filename.rfind('.');
	// pos != 0 is for dotfiles
	if(pos != std::string::npos && pos != 0)
		return filename.substr(0, pos);
	return filename;
}


std::string
GetFileExtension(const std::string filename)
{
	size_t pos = filename.rfind('.');
	// pos != 0 is for dotfiles
	if(pos != std::string::npos && pos != 0)
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


/**
 * Splits command line argument in format a/b/file:10:92 into filename, line
 * and column. If column or line are missing -1 is returned in their place.
 */
std::string
ParseFileArgument(const std::string argument, int32* line, int32* column)
{
	std::string filename;
	if(line != nullptr)
		*line = -1;
	if(column != nullptr)
		*column = -1;
	// first :
	int32 first = argument.find(':');
	if(first != std::string::npos) {
		filename = argument.substr(0, first);
		// second :
		int32 second = argument.find(':', first + 1);
		if(line != nullptr) {
			*line = std::stoi(argument.substr(first + 1, second));
				// if second is npos substr copies to the end
		}
		if(column != nullptr && second != std::string::npos) {
			*column = std::stoi(argument.substr(second + 1));
		}
	} else {
		filename = argument;
	}
	return filename;
}


template<typename T>
bool IsChecked(T* control)
{
	return control->Value() == B_CONTROL_ON;
}
template bool IsChecked<BCheckBox>(BCheckBox*);
template bool IsChecked<BRadioButton>(BRadioButton*);


template<typename T>
void SetChecked(T* control, bool checked)
{
	control->SetValue(checked ? B_CONTROL_ON : B_CONTROL_OFF);
}
template void SetChecked<BCheckBox>(BCheckBox*, bool);
template void SetChecked<BRadioButton>(BRadioButton*, bool);


void
OKAlert(const char* title, const char* message, alert_type type)
{
	BAlert* alert = new BAlert(title, message, B_TRANSLATE("OK"),
		nullptr, nullptr, B_WIDTH_AS_USUAL, type);
	alert->SetShortcut(0, B_ESCAPE);
	alert->Go();
}


int32 rgb_colorToSciColor(rgb_color color)
{
	return color.red | (color.green << 8) | (color.blue << 16);
}


KeyDownMessageFilter::KeyDownMessageFilter(uint32 commandToSend, char key,
	uint32 modifiers)
	:
	BMessageFilter(B_KEY_DOWN),
	fKey(key),
	fModifiers(modifiers),
	fCommandToSend(commandToSend)
{
}


filter_result
KeyDownMessageFilter::Filter(BMessage* message, BHandler** target)
{
	if(message->what == B_KEY_DOWN) {
		const char* bytes;
		uint32 modifiers;
		message->FindString("bytes", &bytes);
		modifiers = static_cast<uint32>(message->GetInt32("modifiers", 0));
		if(bytes[0] == fKey && modifiers == fModifiers) {
			Looper()->PostMessage(fCommandToSend);
			return B_SKIP_MESSAGE;
		}
	}
	return B_DISPATCH_MESSAGE;
}


template<>
void
copy_value(const void* source, ssize_t size, std::string* destination)
{
	destination->resize(size);
	*destination = reinterpret_cast<const char*>(source);
}
