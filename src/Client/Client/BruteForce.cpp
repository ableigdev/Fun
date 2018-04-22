#include "BruteForce.h"
#include <vector>
#include <iostream>
#include <Windows.h>

BruteForce::BruteForce()
	: m_PasswordLength(0)
{

}

void BruteForce::setAlphabet(const std::string& str)
{
	m_Alphabet = str;
}

void BruteForce::setPasswordLength(short int value)
{
	m_PasswordLength = value > 0 ? value : 0;
}

short int BruteForce::getPasswordLength() const
{
	return m_PasswordLength;
}

void BruteForce::setLogin(const std::string& str)
{
	m_Login = str;
}

std::string BruteForce::getLogin() const
{
	return m_Login;
}

std::string BruteForce::getAlphabet(int value)
{
	switch (value)
	{
		case 1:
		{
			return "qwertyuiopasdfghjklzxcvbnm";
		}
		case 2:
		{
			return "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM1234567890";
		}
		case 3:
		{
			return "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM1234567890"\
				"éöóêåíãøùçõúôûâàïğîëäæıÿ÷ñìèòüáş¸ÉÖÓÊÅÍÃØÙÇÕÚÔÛÂÀÏĞÎËÄÆİß×ÑÌÈÒÜÁŞ¨";
		}
		default:
		{
			return{};
		}
	}
}

void BruteForce::brute(CPipeClient<char>& PC)
{
	std::vector<int> indexer{};
	indexer.resize(m_PasswordLength);

	std::string currentPassword{};
	currentPassword.resize(m_PasswordLength);

	int alphabetSize = m_Alphabet.size();

	while (PC.IsPipeConnected())
	{
		for (int i = m_PasswordLength - 1; i >= 0; --i)
		{
			if (i != 0)
			{
				if (indexer[i] == alphabetSize)
				{
					indexer[i] = 0;
					++indexer[i - 1];
				}
			}
		}

		for (int i = 0; i < m_PasswordLength; ++i)
		{
			currentPassword[i] = m_Alphabet[indexer[i]];
		}

		Sleep(2000);
		if (PC.authorization(m_Login, currentPassword) == 1)
		{
			break;
		}

		for (int i = 0; i < m_PasswordLength; ++i)
		{
			if (indexer[i] != alphabetSize - 1)
			{
				break;
			}
		}

		++indexer[m_PasswordLength - 1];
	}
}