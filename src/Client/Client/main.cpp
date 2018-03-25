#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <conio.h>
#include <iostream>
#include <fstream>
#include <string>

#include "PipeClient.h"

#define	PIPE_NAME_PREFIX	"\\\\"
#define	PIPE_NAME			"\\pipe\\pipe_example"
#define MAX_LOG_PASS_LENGTH 50

int main()
{
	char* login = new char[MAX_LOG_PASS_LENGTH];
	char* password = new char[MAX_LOG_PASS_LENGTH];
	char *FName = new char[MAX_PATH], answer;
	bool menuExit = false;
	int alphType = 0;

	char *ServerName = new char[MAX_PATH], *PipeName = new char[MAX_PATH];

	SetConsoleOutputCP(1251);

	std::cout << "������� ��� ������� ( . - ��� ���������� ����������): ";
	std::cin >> ServerName;

	CPipeClient<char*> PC;
	strcat(strcat(strcpy(PipeName, PIPE_NAME_PREFIX), ServerName), PIPE_NAME);
	

	do
	{
		
		std::cout << "\n��������� ������� � ������: \n"
					<< "1) �������� ����� \n"
					<< "2) ������ \n"
					<< "3) ��������� ������ �������\n"
					<< "�������: ";
		int mode = 0;
		std::cin >> mode;

		switch (mode)
		{
		case 1:
			
			std::cout << "\n������� �����: ";
			std::cin >> login;

			std::cout << "������� ������: ";
			std::cin >> password;
		
			PC.ConnectToServer(PipeName, login, password);
			break;

		case 2:
			std::cout << "\n��� �������� ������: \n"
				<< "1) ��������� ��������� ����� - 25 �������� (�� ���������)\n"
				<< "2) ��������� � ������� ��������� ����� + ����� - 62 �������\n"
				<< "3) ��������� � ������� ��������� ����� + ����� + ��������� � ������� ������� ����� - 128 ��������";

			std::cin >> alphType;
			if (alphType > 3 || alphType < 1)
			{
				std::cout << "\n������ ��� �� ���������. \n";
				alphType = 1;
			}
			
			//PC.bruteForce(/*alphabet type*/ alphType

			PC.ConnectToServer(PipeName, login, password);
			break;

		case 3:
			menuExit = true;
			break;

		default:
			std::cout << "\n������ �����, ������� ��� ���.\n";
			break;
		}

	} while (!menuExit);

	delete[]ServerName;
	delete[]PipeName;

	delete[] login;
	delete[] password;

	return 0;
}


