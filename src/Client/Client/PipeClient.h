#pragma once

#include <windows.h>
#include <cstdlib>
#include <string>

template <typename T> 
class CPipeClient
{

	/*
	�������� ������������ ������������ ����, � ������� ����� ��������� ���������� ������������ ������
	*/


	HANDLE hPipe;

	//------------------------------------------------------------------

public:

	CPipeClient()
	{
		hPipe = INVALID_HANDLE_VALUE;
	}


	~CPipeClient()
	{
		if (hPipe != INVALID_HANDLE_VALUE)
			CloseHandle(hPipe);
	}

	//------------------------------------------------------------------
	/*
	��������� ������������ �����, � ������� �������� �������������� ������� ��������
	������������ ������ � ��������� ������ (�������� PipeName) � ������� ������� CreateFile
	��� ������ � ���� ������. ����� � ������ ������������� ����������� �������� ����������
	������� � ������� ���������� ���������� ������� (���� �������� WaitInfinite ����� false,
	�� ���������� ����������� ��������, � � ��������� ������ �������� ����������� � �������
	�������, ���������� ��� �������� ���������� ������ ��������).
	*/

	bool ConnectPipe(char *PipeName, bool WaitInfinite = false)
	{
		/*
		����������� ���� ������� �����������
		*/
		do
		{
			/*
			������� ����������� � ������
			*/
			size_t sizePipe = strlen(PipeName) + 1;

			std::wstring wPipeName(sizePipe, L'#');
			mbstowcs(&wPipeName[0], PipeName, sizePipe);

			hPipe = CreateFile(wPipeName.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				0,
				NULL,
				OPEN_EXISTING,
				0,
				NULL
			);

			if (!IsPipeConnected())
			{
				if (GetLastError() == ERROR_PIPE_BUSY)
				{
					/*
					���� ����������� �� ��������� �� ������� ��������� �������, �� ���������� �������� ���
					������������ (� ������ ����������� ��������   �����) � ������� �� ��������� �����������
					*/

					if (!WaitNamedPipe(wPipeName.c_str(), WaitInfinite ? NMPWAIT_WAIT_FOREVER : NMPWAIT_USE_DEFAULT_WAIT))
						return false;
				}
				else
					// ������ ������ ��� �����������, �������������, ��� �� �������
					return false;
			}
			else
				// ������� �����������
				break;
		} while (1);

		return true;
	}

	//------------------------------------------------------------------
	/*
	��������� ������������ �����, � ������� �������� �������������� ������� �������� ������,
	���� ����������� � ���� ��������� �������, � ����� ���������
	*/

	bool InitMessageMode()
	{
		if (IsPipeConnected())
		{
			DWORD Mode = PIPE_READMODE_MESSAGE;
			return SetNamedPipeHandleState(hPipe, &Mode, NULL, NULL) == TRUE;
		}
		return false;
	}

	//------------------------------------------------------------------
	/*
	��������� ������������ �����, � ������� �������� �������������� ������� ������ ������ �
	�����, ���� ����������� � ���� ��������� �������.
	*/

	bool WriteMessage(std::basic_string<T> &Message)
	{
		if (IsPipeConnected())
		{
			DWORD NBWr;
			return WriteFile(hPipe, &Message.at(0), sizeof(Message), &NBWr, NULL) == TRUE;
		}
		return false;
	}

	//------------------------------------------------------------------

	short int ReadResponse()
	{
		if (IsPipeConnected())
		{
			DWORD NBytesRead;
			short int Message;
            if (ReadFile(hPipe, &Message, sizeof(Message), &NBytesRead, NULL) == TRUE)
            {
                return Message;
            }
		}
		return -1;
	}
	/*
	��������� ������������ �����, � ������� �������� �������������� �������� ���������
	����������� � ���������� ������������ ������
	*/

	bool IsPipeConnected()
	{
		return hPipe != INVALID_HANDLE_VALUE;
	}

	//------------------------------------------------------------------
	/*
	��������� ������������ �����, � ������� �������� �������������� ����������� � ������� � ��������
	*/


	void ConnectToServer(char *PipeName, std::basic_string<T>& login, std::basic_string<T>& password)
	{
		if (ConnectPipe(PipeName))
		{
			InitMessageMode();

			do//����� ������� ����� � ������ ���� �� ������� �������� �����������
			{
				std::cout << "\n������� �����: ";
				std::cin >> login;

				std::cout << "������� ������: ";
				std::cin >> password;
				//InitMessageMode();

				std::basic_string<T> str(login + "/" + password);

				if (!(WriteMessage(str)))
				{
					std::cout << "\n������ ������ � ����������� �����!\n";
				}

				switch (ReadResponse())
				{
					case 0:
					{
						std::cout << "\n�������� ������ ��� �����!\n";
						std::cout << "��������� ����? (N - ���������� ����)";
						char answ = 'y';
						std::cin >> answ;
						if (answ == 'N' || answ == 'n')
						{
							break;
						}
						break;
					}

					case 1:
					{
						std::cout << "\n����������� ������ �������!\n";
						hPipe = INVALID_HANDLE_VALUE;
						break;
					}

					case -1:
					{
						std::cout << "\n���������� ������� ����������� ���������!"  
							<< "\n����� � �������� ��������!" << std::endl;
						
						hPipe = INVALID_HANDLE_VALUE;
						break;
					}
					default:
					{
						std::cout << "\n����������� ������!" << std::endl;
						break;
					}
				}
				
			} while (IsPipeConnected());

			std::cout << "\n������ � �������� ���������.\n";
		}
		else
			std::cout << "\n������ ���������� � �������� (��� ������: " << GetLastError() << ")!\n";

	}

	//------------------------------------------------------------------
	/*
	��������� ������������ �����, � ������� �������� �������������� ������� �����������
	*/

	bool authorization(bool serverAnswer = 0)
	{
		
	}

};