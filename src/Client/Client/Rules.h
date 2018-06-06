#pragma once

#include <string>

template <typename T>
class Rules
{
public:
	Rules(const std::basic_string<T>& = "");

	void setString(const std::basic_string<T>&);
	std::basic_string<T> getString() const;

private:
	std::basic_string<T> m_Str;
};

template <typename T>
Rules<T>::Rules(const std::basic_string<T>& value)
	: m_Str(value)
{

}

template <typename T>
void Rules<T>::setString(const std::basic_string<T>& value)
{
	m_Str = value;
}

template <typename T>
std::basic_string<T> Rules<T>::getString() const
{
	return m_Str;
}