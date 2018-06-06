#pragma once

#include <string>
#include "PipeClient.h"

class BruteForce
{
public:
	BruteForce();

	void setAlphabet(const std::string&);
	std::string getAlphabet(int);

	void setLogin(const std::string&);
	std::string getLogin() const;

	void setPasswordLength(short int);
	short int getPasswordLength() const;

	void brute(CPipeClient<char>&);

private:
	std::string m_Alphabet{};
	short int m_PasswordLength;
	std::string m_Login{};
};