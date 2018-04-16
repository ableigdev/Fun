#pragma once

#include <string>

class BruteForce
{
public:
	BruteForce();

	void setAlphabet(const std::string&);
	void setPassword(const std::string&);

private:
	std::string m_Alphabet;
	std::string m_Password;
};