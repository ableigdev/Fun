#pragma once

#include <windows.h>
#include <cstdlib>
#include <string>
#include <ctime> 

template <typename T> 
class CPipeClient
{

	/*
	�������� ������������ ������������ ����, � ������� ����� ��������� ���������� ������������ ������
	*/


	HANDLE hPipe;

    OVERLAPPED Overl;
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
				FILE_FLAG_OVERLAPPED,
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

	bool WriteMessage(const std::basic_string<T> &Message)
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
            short int Message = -13108; // 
            HANDLE osReadOperation;

            unsigned int end_time = clock() + 3000;

            while (Message < -2)
            {
                ReadFile(hPipe, &Message, sizeof(Message), &NBytesRead, &Overl);
                if (clock() > end_time)
                {
                    return -2;
                }
            }
            
           
            if (Message < 2 && Message > -2)
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


	int authorization(const std::basic_string<T>& login, const std::basic_string<T>& password)
	{
		std::basic_string<T> str(login + "/" + password);

        //HANDLE hevent = CreateEvent(NULL, FALSE, FALSE, NULL);

		if (!(WriteMessage(str)))
		{
			std::cout << "\n������ ������ � ����������� �����!\n";
		}

        unsigned int end_time = clock() + 3000;
        int attempts = 0;
        while (attempts < 3)
        {
            int tmp_debug = ReadResponse();
            switch (tmp_debug)
            {
                case 0:
                {
                    std::cout << "\n�������� ������ ��� �����!\n";
                    return 0;
                }

                case 1:
                {
                    std::cout << "\n����������� ������ �������!\n";
                    hPipe = INVALID_HANDLE_VALUE;
                    return 1;
                }

                case -1:
                {
                    std::cout << "\n���������� ������� ����������� ���������!" << std::endl;

                    hPipe = INVALID_HANDLE_VALUE;
                    
                    return -1;
                }
                case -2:
                { // case of async reading data from server
                    /*if (clock() > end_time)
                    {
                        WriteMessage(str);
                        end_time = clock() + 3000;
                        std::cout << "��� ������ �� �������, ������� �" << attempts << std::endl;
                        attempts++;
                    }
                    Sleep(500 * attempts);
                    break;*/
                    WriteMessage(str);
                    attempts++;
                    Sleep(500 * attempts);
                    std::cout << "��� ������ �� �������, ������� �" << attempts << std::endl;
                    break;
                }
                default:
                {
                    std::cout << "\n����������� ������!" << std::endl;
                    
                }
            }

        }
        std::cout << "\n������ �� ��������! ���������� ���������.\n" << std::endl;
        hPipe = INVALID_HANDLE_VALUE;
        return -1;
        //std::cout << "������ ���������� � ��������! ���������� ���������\n";
        
	}

	//------------------------------------------------------------------

};