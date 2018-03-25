#include <iostream>
#include <fstream>
#include <Windows.h>

#include "PipeServer.h"
#include "PerPipeStruct.h"

#define MAX_PIPE_INST	2
#define PIPE_NAME		L"\\\\.\\pipe\\pipe_example"

int main()
{
	/*
	�������� ����������:
	hEvents - ������ �� MAX_PIPE_INST ���������, ���������� ����������� �������;
	PipeInfo - ������ �� MAX_PIPE_INST ���������, ���������� ���������� ������� PerPipeStruct ��� ������� ���������� ������;
	Pipes - ������ �� MAX_PIPE_INST ���������, ���������� ���������� ������� CPipeServer ��� ������� ���������� ������;
	FName - ������, � ������� ����� ������� ������������� ��� �����;
	answer - ����������, � ������� ����� ���������������� ������� ����� ������ ����� ������������ ��� ������������� ����������� ������ �������;
	file - ��������� ������ ofstream, � ������� �������� ����� �������������� ������ � ����;
	PipeNumber - ����� ���������� ������, ��� �������� ����������� ����������� ��������;
	NBytesRead - ���������� ����������� �� ������ ����;
	Message - ����������� �� ������ ��������� (���������� �� ������ ������ ������ �������� ��� ������ ���������);
	PipesConnect - ������������� ���������� ���������� ������������ ������������ �������� (��� ����������� ���������� ������� ������������� �� 1, ��� ���������� - ����������� �� 1).
	*/


	HANDLE hEvents[MAX_PIPE_INST];
	PerPipeStruct<char> PipeInfo[MAX_PIPE_INST];
	CPipeServer<char> Pipes[MAX_PIPE_INST];
	char *FName = new char[MAX_PATH], answer;
	std::ofstream file;
	DWORD PipeNumber, NBytesRead;
	char* Message = new char[100];
	int PipesConnect = 0;

	SetConsoleOutputCP(1251);
	std::cout << "������� ��� ��������� �����: ";
	std::cin >> FName;

	file.open(FName);
	if (!file)
		std::cout << "������ �������� ����� � ������ " << FName << "!" << std::endl;
	else
	{
		for (int i = 0; i < MAX_PIPE_INST; i++)
		{

			/*
			��������� ������������ ����������� ������� � �������� ����������� ����������� ������� �
			������� �� � ����� �������� ����������� �������
			*/

			hEvents[i] = CreateEvent(NULL, TRUE, TRUE, NULL);
			Pipes[i].CreatePipeAndWaitClient(PIPE_NAME, hEvents[i]);
			if (Pipes[i].GetState() == PIPE_ERROR)
			{

				/*
				���� ��������� ������ ��� �������� ���������� ������������ ������, �� ������������
				���� ���������� � ��������� �������� � ���������� ����������
				*/

				file.close();
				delete[]FName;

				for (i--; i >= 0; i--)
					CloseHandle(hEvents[i]);

				std::wcout << "������ �������� ����������� ������������ ������ " << PIPE_NAME << std::endl;

				return 0;
			}
		}

		std::cout << "�������� ����������� ��������..." << std::endl;
		/*
		����������� ����, ������������ ���������� ���� �������
		*/

		do
		{

			/*
			�������� �������� � ��������� ��������� �������, ���������� � �����-�� �� �������,
			��� ��������������� � ���������� ����������� ��������, � ��������� ������ ����� ������.
			��� ���� ������� WaitForMultipleObjects ��������� ������� � ������ �������
			������������� �� ��������.
			*/

			PipeNumber = WaitForMultipleObjects(MAX_PIPE_INST, hEvents, FALSE, INFINITE) - WAIT_OBJECT_0;

			if (PipeNumber < MAX_PIPE_INST)
			{

				/*
				���� ������� ���������� �����, �� �����������, ���� �� �������� ����������� ��������, �,
				���� ��� ���� ��������, �� ����������� ��������� ����������� ����������� �������� �������
				GetPendingResult � ����������� ��������������� ��������� ����� ��������� ���������� ������
				*/

				if (!Pipes[PipeNumber].GetIOComplete())
					Pipes[PipeNumber].GetPendingResult(NBytesRead);

				if (Pipes[PipeNumber].GetIOComplete())
				{
					/*
					���� ���������� ������� ���������� ����������� �������� ��� ���������� ���������� ������,
					�� �������� ���� ��� ���������
					*/

					switch (Pipes[PipeNumber].GetState())
					{

						/*
						� ������, ����� ��� ������ � ������� ���������� �������������� ������
						*/

					case PIPE_ERROR:			
						std::cout << "������ ��� ������ � �������! ������������ �������������� ������������ ������� (��� ������: " 
							<< GetLastError() << ")!" << std::endl;

						/*
						���� ������ ���������, �� �������� ����, ����������� �� �� ������ ��� (���������������
						�������� � ���� ��������� �������� � ������), �, ���� ��� ���, �� ���������� ����������
						������������ �������� �� 1.
						*/
					case PIPE_CONNECTED:		
						if (Pipes[PipeNumber].GetOperState() == PIPE_JUST_CONNECTED)
						{
							std::cout << "Testing Message. Just connected" << std::endl;
							PipesConnect++;
						}

						if (Pipes[PipeNumber].ReadMessage(Message))
						{
							/*
							���� ��������� ����������� �������� ������, �� �������� ��������� ��������
							*/

							switch (Pipes[PipeNumber].GetOperState())
							{
								/*
								���� ��������� �� ��� ������ ���������, �� ������ ���������� ������
								*/

							case PIPE_READ_PART:		Pipes[PipeNumber].ReadMessage(Message);
								std::cout << "Testing Message. Reading data part" << std::endl;
								break;

								/*
								���� ������ ��������� ��������� �������, �� ��������� ����������� ��������
								*/

							case PIPE_READ_SUCCESS:		PipeInfo[PipeNumber].ReadVal(Message);
								std::cout << "Testing Message. Reading data" << std::endl;
								break;

								/*
								��������� ������ ������
								*/
							case PIPE_OPERATION_ERROR:	std::cout << "������ ��� ������ ������ �� ������ (��� ������: " << GetLastError() << ")!" << std::endl;
								break;

							}
							break;
						}
						//break;

						/*
						���������� �������. � ���� ������ ���������� ����� ������, ����������� �� ������ � ���� �
						�� ������� � ��������� ������, ���������� ���������� ������������ �������� �� 1,
						���������� ������� �� ������� ������� � ������ �������� ����������� ������ �������
						*/

					case PIPE_LOST_CONNECT:		
						file << PipeInfo[PipeNumber];
						std::cout << "Testing Message. File Write" << std::endl;
						PipeInfo[PipeNumber].ClearData();
						if (PipesConnect > 0)
							PipesConnect--;
						Pipes[PipeNumber].DisconnectClient();
						Pipes[PipeNumber].WaitClient();
						if (Pipes[PipeNumber].CanClose() == false)
						{
							std::cout << "� ����� �� ���� �������� ������� ������ �� ������� �������. ��������� ������� ������ ������Y ��� y - �� / ����� ������ ������� - ���)?" << std::endl;
							std::cin >> answer;
							if (answer == 'Y' || answer == 'y')
								continue;
						}
						break;

					}
				}
			}

			/*
			���� ��� ������������ ��������, �� ������ � ������������� ����������� ������ � ����������
			��������������� ��������
			*/


			if (PipesConnect == 0)
			{
				std::cout << "��� ������� ���������! ���������� ������ (Y ��� y - �� / ����� ������ ������� - ���)? ";
				std::cin >> answer;
				if (answer != 'Y' && answer != 'y')
					break;
				std::cout << "�������� ����������� ��������..." << std::endl;
			}

		} while (1);

		/*
		������������ ���������� � ��������� ��������
		*/

		for (int i = 0; i < MAX_PIPE_INST; i++)
			CloseHandle(hEvents[i]);

		file.close();
	}

	delete[]FName;

	return 0;
}