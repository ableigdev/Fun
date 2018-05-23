#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <conio.h>
#include <iostream>
#include <fstream>
#include <string>

#define	PIPE_NAME_PREFIX	"\\\\"
#define	PIPE_NAME			"\\pipe\\pipe_example"
#define MAX_LOG_PASS_LENGTH 50

#include "PipeClient.h"
#include "BruteForce.h"

void errCodeOut(int);

int main()
{
	std::basic_string<char> login{};
	std::basic_string<char> password{};

	char *FName = new char[MAX_PATH];
	bool menuExit = false;
	int alphType = 0;

	char *ServerName = new char[MAX_PATH];
	char *PipeName = new char[MAX_PATH];

	SetConsoleOutputCP(1251);

	std::cout << "Введите имя сервера ( . - для локального компьютера): ";
	std::cin >> ServerName;

	CPipeClient<char> PC;
	BruteForce bruteForce;
	std::string alphabet{};
	short int maxPasswordLength = 0;

	strcat(strcat(strcpy(PipeName, PIPE_NAME_PREFIX), ServerName), PIPE_NAME);
	std::cout << PipeName << std::endl;

	do
	{
		std::cout << "\nЗапустить клиента в режиме: \n"
					<< "1) Проверки связи \n"
					<< "2) Взлома \n"
					<< "3) Завершить работу клиента\n"
					<< "Выбрано: ";
		int mode;
		std::cin >> mode;

        if (PC.ConnectPipe(PipeName))
        {
            PC.InitMessageMode();

            switch (mode)
            {
            case 1:
                //передаем пустые логин и пароль
                if (PC.ConnectPipe(PipeName))
                {
                    PC.InitMessageMode();
                    do
                    {
                        std::cout << "\nВведите логин: ";
                        std::cin >> login;

                        std::cout << "Введите пароль: ";
                        std::cin >> password;

                        if (PC.authorization(login, password) == 0)
                        {
                            char answer;
                            std::cout << "Повторить ввод? (Y/N): ";
                            std::cin >> answer;
                            if (answer == 'N' || answer == 'n')
                            {
                                PC.WriteMessage("C");
                                break;
                            }
                        }
                    } while (PC.IsPipeConnected());
                }
                else
                {
                    std::cout << "\n?????? ?????????? ? ???????? (??? ??????: " << GetLastError() << ")!\n";
                }
                break;

            case 2:
                std::cout << "\nТип алфавита пароля: \n"
                    << "1) Маленькие латинские буквы - 25 символов (по умолчанию)\n"
                    << "2) Маленькие и большие латинские буквы + цифры - 62 символа\n"
                    << "3) Маленькие и большие латинские цифры + цифры + маленькие и большие русские буквы - 128 символов" << std::endl;

                std::cin >> alphType;
                if (alphType > 3 || alphType < 1)
                {
                    std::cout << "\nВыбран тип по умолчанию. \n";
                    alphType = 1;
                }

                alphabet = bruteForce.getAlphabet(alphType);

                std::cout << "Введите максимально допустимую длину пароля: ";
                std::cin >> maxPasswordLength;

                std::cout << "Введите логин: ";
                std::cin >> login;

                bruteForce.setLogin(login);
                bruteForce.setAlphabet(alphabet);
                bruteForce.setPasswordLength(maxPasswordLength);


                PC.InitMessageMode();

                std::cout << "\n<---Взлом стартовал--->" << std::endl;
                bruteForce.brute(PC);


                break;

            case 3:
                menuExit = true;
                break;

            default:
                std::cout << "\nОшибка ввода, введите еще раз.\n";
                break;
            }
        }
        std::cout << "\nРабота с сервером завершена.\n";

	} while (!menuExit);

	delete[]ServerName;
	delete[]PipeName;

	return 0;
}


void errCodeOut(int switch_on)
{
    std::cout << "\nОшибка соединения с сервером (код ошибки: ";
    switch (switch_on)
    {
    case 53:
        std::cout << switch_on <<" (ERROR_BAD_NETPATH)";
        break;
    case 231:
        std::cout << switch_on << " (ERROR_PIPE_BUSY)";
        break;



    default:
        std::cout << switch_on;
        break;
    }
    std::cout << ")!\n";;
}