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

	std::cout << "������� ��� ������� ( . - ��� ���������� ����������): ";
	std::cin >> ServerName;

	CPipeClient<char> PC;
	BruteForce bruteForce;
	std::string alphabet{};
	short int maxPasswordLength = 0;

	strcat(strcat(strcpy(PipeName, PIPE_NAME_PREFIX), ServerName), PIPE_NAME);
	std::cout << PipeName << std::endl;

	do
	{
		std::cout << "\n��������� ������� � ������: \n"
					<< "1) �������� ����� \n"
					<< "2) ������ \n"
					<< "3) ��������� ������ �������\n"
					<< "�������: ";
		int mode;
		std::cin >> mode;

        switch (mode)
        {
        case 1:
            //�������� ������ ����� � ������
            if (PC.ConnectPipe(PipeName))
            {
                PC.InitMessageMode();
                do
                {
                    std::cout << "\n������� �����: ";
                    std::cin >> login;

                    std::cout << "������� ������: ";
                    std::cin >> password;

                    if (PC.authorization(login, password) == 0)
                    {
                        char answer;
                        std::cout << "��������� ����? (Y/N): ";
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
                errCodeOut(GetLastError());
                //std::cout << "\n������ ���������� � �������� (��� ������: " << GetLastError() << ")!\n";
            }
            break;


        case 2:
            std::cout << "\n��� �������� ������: \n"
                << "1) ��������� ��������� ����� - 25 �������� (�� ���������)\n"
                << "2) ��������� � ������� ��������� ����� + ����� - 62 �������\n"
                << "3) ��������� � ������� ��������� ����� + ����� + ��������� � ������� ������� ����� - 128 ��������" << std::endl;

            std::cin >> alphType;
            if (alphType > 3 || alphType < 1)
            {
                std::cout << "\n������ ��� �� ���������. \n";
                alphType = 1;
            }

            alphabet = bruteForce.getAlphabet(alphType);

            std::cout << "������� ����������� ���������� ����� ������: ";
            std::cin >> maxPasswordLength;

            std::cout << "������� �����: ";
            std::cin >> login;

            bruteForce.setLogin(login);
            bruteForce.setAlphabet(alphabet);
            bruteForce.setPasswordLength(maxPasswordLength);


            if (PC.ConnectPipe(PipeName))
            {
                PC.InitMessageMode();

                std::cout << "\n<---����� ���������--->" << std::endl;
                bruteForce.brute(PC);
                menuExit = true;
            }

            break;

        case 3:
            menuExit = true;
            break;

        default:
            std::cout << "\n������ �����, ������� ��� ���.\n";
            break;
        }
        std::cout << "\n������ � �������� ���������.\n";

	} while (!menuExit);

	delete[]ServerName;
	delete[]PipeName;
    system("pause");
	return 0;
}


void errCodeOut(int switch_on)
{
    std::cout << "\n������ ���������� � �������� (��� ������: ";
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