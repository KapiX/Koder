/*
 * Copyright 2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Utils.h"

#include <Application.h>
#include <Bitmap.h>
#include <CheckBox.h>
#include <IconUtils.h>
#include <MessageFilter.h>
#include <RadioButton.h>
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
