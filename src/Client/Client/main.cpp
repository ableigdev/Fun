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

	std::cout << "Введите имя сервера ( . - для локального компьютера): ";
	std::cin >> ServerName;

	CPipeClient<char*> PC;
	strcat(strcat(strcpy(PipeName, PIPE_NAME_PREFIX), ServerName), PIPE_NAME);
	

	do
	{
		
		std::cout << "\nЗапустить клиента в режиме: \n"
					<< "1) Проверки связи \n"
					<< "2) Взлома \n"
					<< "3) Завершить работу клиента\n"
					<< "Выбрано: ";
		int mode = 0;
		std::cin >> mode;

		switch (mode)
		{
		case 1:
			
			std::cout << "\nВведите логин: ";
			std::cin >> login;

			std::cout << "Введите пароль: ";
			std::cin >> password;
		
			PC.ConnectToServer(PipeName, login, password);
			break;

		case 2:
			std::cout << "\nТип алфавита пароля: \n"
				<< "1) Маленькие латинские буквы - 25 символов (по умолчанию)\n"
				<< "2) Маленькие и большие латинские буквы + цифры - 62 символа\n"
				<< "3) Маленькие и большие латинские цифры + цифры + маленькие и большие русские буквы - 128 символов";

			std::cin >> alphType;
			if (alphType > 3 || alphType < 1)
			{
				std::cout << "\nВыбран тип по умолчанию. \n";
				alphType = 1;
			}
			
			//PC.bruteForce(/*alphabet type*/ alphType

			PC.ConnectToServer(PipeName, login, password);
			break;

		case 3:
			menuExit = true;
			break;

		default:
			std::cout << "\nОшибка ввода, введите еще раз.\n";
			break;
		}

	} while (!menuExit);

	delete[]ServerName;
	delete[]PipeName;

	delete[] login;
	delete[] password;

	return 0;
}


