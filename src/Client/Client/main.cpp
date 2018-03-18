#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <conio.h>
#include <iostream>

#include "PipeClient.h"

#define	PIPE_NAME_PREFIX	"\\\\"
#define	PIPE_NAME			"\\pipe\\pipe_example"

//------------------------------------------------

template <class T> T get_start(T Value, T Divider)
{
	T Rem = Value % Divider;

	return Rem == 0 ? Value : Value + Divider - Rem;
}

//------------------------------------------------

int main()
{
	unsigned StartVal, EndVal, Divider;

	SetConsoleOutputCP(1251);

	std::cout << "������� ��������� ��������: ";
	std::cin >> StartVal;

	std::cout << "������� �������� ��������: ";
	std::cin >> EndVal;

	std::cout << "������� ��������, �� ������� ����� ������ �������� ��� �������: ";
	std::cin >> Divider;

	CPipeClient<unsigned> PC;
	char *ServerName = new char[MAX_PATH], *PipeName = new char[MAX_PATH];
	
	std::cout << "������� ��� ������� (. - ��� ���������� ����������): ";
	std::cin >> ServerName;

	strcat(strcat(strcpy(PipeName, PIPE_NAME_PREFIX), ServerName), PIPE_NAME);

	if (PC.ConnectPipe(PipeName))
	{
		PC.InitMessageMode();
		StartVal = get_start(StartVal, Divider);
		if (PC.WriteMessage(StartVal) && PC.WriteMessage(EndVal) && PC.WriteMessage(Divider))
		{
			for (; StartVal <= EndVal; StartVal += Divider)
			{
				if (!PC.WriteMessage(StartVal))
				{
					std::cout << "������ ������ � ����������� �����!\n";
					break;
				}
			}
		}
		else
			std::cout << "������ ������ � ����������� �����!\n";

		std::cout << "������� ����� ������� ��� ���������� ��������� (���������� ���������� �� ������������ ������ " << PipeName << ")\n";
	}
	else
		std::cout << "������ ���������� � �������� (��� ������: " << GetLastError() << ")! ������� ����� ������� ��� ������\n";

	delete[]ServerName;
	delete[]PipeName;

	std::cin.get();

	return 0;
}
