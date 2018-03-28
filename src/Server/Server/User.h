#pragma once

#include <string>

template <typename T>
struct User
{
	std::basic_string<T> login;
	std::basic_string<T> password;
};