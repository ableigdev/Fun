#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <conio.h>
#include <iostream>
#include <string>

#include "PipeClient.h"

#define	PIPE_NAME_PREFIX	"\\\\"
#define	PIPE_NAME			"\\pipe\\pipe_example"
#define MAX_LOG_PASS_LENGTH 50

int main()
{
	char* login = new char[MAX_LOG_PASS_LENGTH];
	char* password = new char[MAX_LOG_PASS_LENGTH];

	SetConsoleOutputCP(1251);

	std::cout << "Введите логин: ";
	std::cin >> login;

	std::cout << "Введите пароль: ";
	std::cin >> password;

	CPipeClient<char*> PC;
	char *ServerName = new char[MAX_PATH], *PipeName = new char[MAX_PATH];
	
	std::cout << "Введите имя сервера (. - для локального компьютера): ";
	std::cin >> ServerName;

	strcat(strcat(strcpy(PipeName, PIPE_NAME_PREFIX), ServerName), PIPE_NAME);

	if (PC.ConnectPipe(PipeName))
	{
		PC.InitMessageMode();
		if (!(PC.WriteMessage(login) && PC.WriteMessage(password)))
		{
			std::cout << "Ошибка записи в именованный канал!\n";
		}
			
		std::cout << "Нажмите любую клавишу для завершения программы (выполнится отключение от именованного канала " << PipeName << ")\n";
	}
	else
		std::cout << "Ошибка соединения с сервером (код ошибки: " << GetLastError() << ")! Нажмите любую клавишу для выхода\n";

	delete[]ServerName;
	delete[]PipeName;

	delete[] login;
	delete[] password;

	system("pause");

	return 0;
}
