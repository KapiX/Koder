/*
 * Copyright 2014-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef STYLER_H
#define STYLER_H


#include <string>
#include <set>

#include <yaml-cpp/yaml.h>


class BPath;
class Editor;


class Styler {
public:
	static	void	ApplyGlobal(Editor* editor, const char* style);
	static	void	ApplyLanguage(Editor* editor, const char* style, const char* lang);

	static	void	GetAvailableStyles(std::set<std::string> &styles);

private:
	static	void	_ApplyGlobal(Editor* editor, const char* style, const BPath &path);
	static	void	_ApplyLanguage(Editor* editor, const char* style, const char* lang, const BPath &path);
	static	void	_GetAvailableStyles(std::set<std::string> &styles, const BPath &path);
	static	void	_GetAttributesFromNode(const YAML::Node &node, int* styleId,
						int* fgColor, int* bgColor, int* fontStyle);
	static	void	_SetAttributesInEditor(Editor* editor, int styleId,
						int fgColor, int bgColor, int fontStyle);
	static	int		_CSSToInt(const std::string cssColor);
};


#endif // STYLER_H
