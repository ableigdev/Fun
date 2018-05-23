#pragma once

#include <windows.h>
#include <cstdlib>
#include <string>
#include <iostream> 
#include <future>
#include <chrono>

template <typename T>
class CPipeClient
{

    /*
    �������� ������������ ������������ ����, � ������� ����� ��������� ���������� ������������ ������
    */

    OVERLAPPED osReadOperation;
    int rec_deep;

    HANDLE hPipe;

    //------------------------------------------------------------------

public:

    CPipeClient()
    {
        hPipe = INVALID_HANDLE_VALUE;
        rec_deep = 0;
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
                //�� ���� ������ � ��������� ����, ��� ��� � ����� ������ ���������� ������. \
                    ����� ���������� ������������ �������, ����� ������ ������?
                /*
                if (GetLastError() == ERROR_PIPE_BUSY)
                {
                    
                    //���� ����������� �� ��������� �� ������� ��������� �������, �� ���������� �������� ���
                    //������������ (� ������ ����������� ��������   �����) � ������� �� ��������� �����������
                    

                    if (!WaitNamedPipe(wPipeName.c_str(), WaitInfinite ? NMPWAIT_WAIT_FOREVER : NMPWAIT_USE_DEFAULT_WAIT))
                        return false;
                }
                else
                    // ������ ������ ��� �����������, �������������, ��� �� �������
                    return false;*/
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
            short int Message;
            
            /*if (ReadFile(hPipe, &Message, sizeof(Message), &NBytesRead, NULL) == TRUE)
            {
                std::cout << NBytesRead << "\t" << Message << "\t\n";
                return Message;
            }*/

            //--------------------------------------------------------------------------
            bool fOverlapped = FALSE;


            if (!ReadFile(hPipe, &Message, sizeof(Message), &NBytesRead, &osReadOperation))
            {
                if (GetLastError() != ERROR_IO_PENDING)
                {
                    // Some other error occurred while reading the file.
                    return -1;
                }
                else
                {
                    fOverlapped = TRUE;
                }
            }
            else
            {
                // Operation has completed immediately.
                fOverlapped = FALSE;
                std::cout << "immediately message " << Message << std::endl;
                return Message;
            }

            if (fOverlapped)
            {
                int time = clock() + 500;
                while (clock() < time)
                {
                    if (GetOverlappedResult(hPipe, &osReadOperation, &NBytesRead, FALSE))
                    {
                        //ReadHasCompleted(NumberOfBytesTransferred);
                        if (Message < -13)
                        {
                            return -2;
                        }
                        return Message;
                    }
                    else
                    {
                        if (GetLastError() == ERROR_IO_INCOMPLETE)
                        {
                            continue;
                        }
                        else
                        {
                            // Operation has completed, but it failed.
                            return -1;
                        }

                    }
                }
                return -2;
                // Wait for the operation to complete before continuing.
                // You could do some background work if you wanted to.
                
            }
            else
            {
                std::cout << NBytesRead << "\t" << Message << "\t\n";
                return Message;
            }
                
            //--------------------------------------------------------------------------
            
            //DWORD avail;
            //bool bSuccess = FALSE;
            //bool tSuccess = FALSE;

            //tSuccess = PeekNamedPipe(hPipe, NULL, 0, NULL, &avail, NULL);

            //if (tSuccess && avail > 0)
            //{

            //if (ReadFile(hPipe, &Message, sizeof(Message), &NBytesRead, NULL) == TRUE)
            //{
              //  return Message;
            //}
            //}
            //std::cout << tSuccess << "\t" << avail << "\t\n";
            

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


    int authorization(const std::basic_string<T>& login, const std::basic_string<T>& password, bool outputFlag = true)
    {
        std::basic_string<T> str(login + "/" + password);

        if (!(WriteMessage(str)))
        {
            std::cout << "\n������ ������ � ����������� �����!\n";
        }
        int res = 0;

        int choose = ReadResponse();
        switch (choose)
        {
        case 0:
        {
            if(outputFlag)
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
            // ���� ������ ���� ������� � �������� �������� ������
            rec_deep++;
            Sleep(5);
            //std::cout << "rec_deep = " << rec_deep << std::endl;
            res = (rec_deep < 5 ? authorization(login, password, false) : -1);
            rec_deep--;
            return res;
        case -3:
            std::cout << "\n������ ������ �� �������! ( " << choose << " )" << std::endl;
            break;
        default:
        {
            std::cout << "\n����������� ������! ( " << choose << " )" << std::endl;
            break;
        }
        }
    }

    //------------------------------------------------------------------

};