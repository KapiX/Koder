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


template<type_code BType>
struct property_type
{
	using type = void*;
};


template<>
struct property_type<B_STRING_TYPE>
{
	using type = std::string;
};


template<typename Dest>
void
copy_value(const void* source, ssize_t, Dest* destination)
{
	*destination = *reinterpret_cast<const Dest*>(source);
}


template<type_code BType>
class message_property
{
private:
	BMessage* message_;
	std::string prop_name_;

public:
	class iterator {
	private:
		int index_;
		BMessage* message_;
		std::string prop_name_;
	public:
		iterator(BMessage* message, std::string prop_name, int index = 0)
			: message_(message), prop_name_(prop_name), index_(index) {}
		bool operator==(iterator &rhs) {
			return index_ == rhs.index_ &&
				message_ == rhs.message_ &&
				prop_name_ == rhs.prop_name_;
		}
		bool operator!=(iterator &rhs) { return !(*this == rhs); }
		iterator &operator++() { ++index_; return *this; }
		iterator operator++(int) {
			iterator clone(*this);
			++index_;
			return clone;
		}
		typename property_type<BType>::type operator*() {
			ssize_t size;
			const void *data;
			status_t status = message_->FindData(prop_name_.c_str(), BType, index_, &data, &size);
			if(status == B_OK) {
				typename property_type<BType>::type value;
				copy_value<typename property_type<BType>::type>(data, size, &value);
				return value;
			}
			// FIXME: throw an exception here
		}
	};
	message_property(BMessage* message, std::string prop_name)
		: message_(message), prop_name_(prop_name) {}
	iterator begin() { return iterator(message_, prop_name_, 0); }
	iterator end() { return iterator(message_, prop_name_, size()); }
	size_t size() {
		int32 count;
		if(message_->GetInfo(prop_name_.c_str(), nullptr, &count) == B_OK) {
			return static_cast<size_t>(count);
		}
		return 0; // FIXME: throw an exception here
	}
};


#endif // UTILS_H
