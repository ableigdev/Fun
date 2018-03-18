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

	std::cout << "Введите начальное значение: ";
	std::cin >> StartVal;

	std::cout << "Введите конечное значение: ";
	std::cin >> EndVal;

	std::cout << "Введите делитель, на который числа должны делиться без остатка: ";
	std::cin >> Divider;

	CPipeClient<unsigned> PC;
	char *ServerName = new char[MAX_PATH], *PipeName = new char[MAX_PATH];
	
	std::cout << "Введите имя сервера (. - для локального компьютера): ";
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
					std::cout << "Ошибка записи в именованный канал!\n";
					break;
				}
			}
		}
		else
			std::cout << "Ошибка записи в именованный канал!\n";

		std::cout << "Нажмите любую клавишу для завершения программы (выполнится отключение от именованного канала " << PipeName << ")\n";
	}
	else
		std::cout << "Ошибка соединения с сервером (код ошибки: " << GetLastError() << ")! Нажмите любую клавишу для выхода\n";

	delete[]ServerName;
	delete[]PipeName;

	std::cin.get();

	return 0;
}
