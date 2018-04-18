#pragma once

#include <string>

class BruteForce
{
public:
	BruteForce();

	void setAlphabet(const std::string&);
	std::string getAlphabet(int);

	void setPasswordLength(short int);
	short int getPasswordLength() const;

	void brute();

private:
	std::string m_Alphabet{};
	short int m_PasswordLength;
};