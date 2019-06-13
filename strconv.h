// strconv.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once


#include <iostream>
#include <locale>
#include <codecvt>

std::string ws2s(const std::wstring& wstr);
std::wstring s2ws(const std::string& str);