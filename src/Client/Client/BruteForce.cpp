#include "BruteForce.h"

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

void BruteForce::brute()
{
	
}