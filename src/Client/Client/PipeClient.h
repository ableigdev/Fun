#pragma once

#include <windows.h>
#include <cstdlib>

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

	bool WriteMessage(T &Message)
	{
		if (IsPipeConnected())
		{
			DWORD NBWr;
			return WriteFile(hPipe, (LPCSTR)(Message), 100, &NBWr, NULL) == TRUE;
		}
		return false;
	}

	//------------------------------------------------------------------

	bool ReadResponse()
	{
		if (IsPipeConnected())
		{
			DWORD NBytesRead;
			bool Message;
			ReadFile(hPipe, &Message, 1, &NBytesRead, NULL);
			
			return Message;
		}
		return false;
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

	void ConnectToServer(char *PipeName)
	{
		char* login = new char[MAX_LOG_PASS_LENGTH];
		char* password = new char[MAX_LOG_PASS_LENGTH];

		if (ConnectPipe(PipeName))
		{
			

			do//����� ������� ����� � ������ ���� �� ������� �������� �����������
			{
				std::cout << "\n������� �����: ";
				std::cin >> login;

				std::cout << "������� ������: ";
				std::cin >> password;
				//InitMessageMode();

				if (!(WriteMessage(login) && WriteMessage(password)))
				{
					std::cout << "\n������ ������ � ����������� �����!\n";
					break;
				}

				if (ReadResponse())
				{
					std::cout << "\n����������� ������ �������!\n";
					break;
				}
				else
				{
					std::cout << "\n�������� ������ ��� �����!\n";
					std::cout << "��������� ����? (N - ���������� ����)";
					char answ = 'y';
					std::cin >> answ; 
					if (answ == 'N' || answ == 'n')
					{
						break;
					}
				}
			} while (IsPipeConnected());

			std::cout << "\n������� ����� ������� ��� ���������� ��������� (���������� ���������� �� ������������ ������ " << PipeName << ")\n";
		}
		else
			std::cout << "\n������ ���������� � �������� (��� ������: " << GetLastError() << ")!\n";

		delete[] login;
		delete[] password;
	}

	//------------------------------------------------------------------
	/*
	��������� ������������ �����, � ������� �������� �������������� ������� �����������
	*/

	bool authorization(bool serverAnswer = 0)
	{
		
	}

};