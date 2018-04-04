#pragma once

#include <windows.h>
#include <vector>
#include <fstream>
#include <string>

#include "User.h"

#define	PIPE_ERROR					-1
#define PIPE_NOT_CONNECTED			1
#define	PIPE_CONNECTED				2
#define	PIPE_LOST_CONNECT			3

#define	PIPE_OPERATION_ERROR		-2
#define	PIPE_JUST_CONNECTED			10
#define	PIPE_NO_OPERATION			11

#define	PIPE_READ_SUCCESS			12
#define	PIPE_READ_PART				13

#define CLIENT_DISCONNECT           109

#define DEF_BUF_SIZE				16384
#define DEF_WAIT_TIME				20000

//------------------------------------------------------------------

template <typename T>
class CPipeServer
{
    // �������� ����������� ������������ ����� ������

    HANDLE readEvent;

    // ���������� ������������ ������

    HANDLE hPipe;

    // ��������� ��������� OVERLAPPED ��� ������������� ������ � ����������� ������
    OVERLAPPED Overl;

    /*
    ����, ���������� ��������� ������������ ������. ��������� ��������:
    PIPE_ERROR - ������ ��� ������ � ����������� �������. ������������� ������ ����������
    PIPE_NOT_CONNECTED - ������ � ������ �� ���������. ������������� ������ ����������
    PIPE_CONNECTED - ������ ����������� � ������. ����� ����� ���� �����������
    PIPE_LOST_CONNECT - ������ ���������� �� ������. ������������� ������ ����������
    */
    DWORD PipeState;

    /*
    ����, ������������ ��������������� ����������� �������� �����-������:
    true - ����������� �������� �����-������ ���������
    false - ����������� �������� �����-������ � �������� ����������
    */
    bool fPendingIOComplete;
    bool CanCloseFlag;
    /*
    ����, ���������� ��������� ������� �������� � ����������� ������:
    PIPE_OPERATION_ERROR - ������ ���������� ��������
    PIPE_JUST_CONNECTED - ������� �������� � ������ �� �����������, �. �. ������ ������ ��� �����������
    PIPE_NO_OPERATION - ������� �������� � ������ �� ����������� ���� ����������� �������� ��� �� ���������
    PIPE_READ_SUCCESS - �������� ������������ ������ ������� ���������
    PIPE_READ_PART - �������� ������������ ������ ������� ���������, ������ ��������� ���� ����� ���������
    */
    DWORD PipeCurOperState;

    /*
    ��������� �� ��������� ������, ����������� ���������� ������������. ��� ���� ����� �
    ������� ������������ ������ ����� ���� ������������ �� ���� ���������� �������
    ��������������� ���������� ������������, ����������� ���� ������������� (�� ����� �������
    ����������� �������) ������������ ������ � ����������� ����� �������. � ��������� ������
    ������������ �� ������ ������� � ������� �� ����� ������ � ���� �� ������������.
    */

    PSECURITY_DESCRIPTOR pSD;

    std::vector<User<T>> db_users;
    int maxLenLog, maxLenPass;

    //------------------------------------------------------------------

    /*
    ����������� ������������ ����� ������, ������� � ����������� �� ����, ������������� �������� GetLastError,
    �������� ���� ��������� ������
    */

    void CheckError()
    {
        DWORD Error = GetLastError();

        switch (Error)
        {
            // ����������� �������� � �������� ����������
        case ERROR_IO_PENDING:			PipeCurOperState = PIPE_NO_OPERATION;
            fPendingIOComplete = false;

            break;

            // ������ ���������� �� ������������ ������
        case ERROR_BROKEN_PIPE:			PipeState = PIPE_LOST_CONNECT;
            fPendingIOComplete = true;
            break;

            // ��������� ������ ��� ������ � ������� 
        default:						PipeCurOperState = PIPE_OPERATION_ERROR;
            fPendingIOComplete = true;
            break;

        }
    }

    //------------------------------------------------------------------

    // ��������� ������������� ����� ���������� ��������� OVERLAPPED
    void ClearOVERL()
    {
        Overl.Offset = Overl.OffsetHigh = Overl.Internal = Overl.InternalHigh = 0;
    }


    //------------------------------------------------------------------

public:

    CPipeServer()
    {
        hPipe = INVALID_HANDLE_VALUE;
        PipeState = PIPE_NOT_CONNECTED;
        PipeCurOperState = PIPE_NO_OPERATION;
        ClearOVERL();
        fPendingIOComplete = true;
        Overl.hEvent = NULL;
        CanCloseFlag = false;
        pSD = NULL;
        maxLenLog = 0;
        maxLenPass = 0;
    }


    ~CPipeServer()
    {
        if (hPipe != INVALID_HANDLE_VALUE)
        {
            // ������ ���� ����������� ��������
            CancelIo(hPipe);
            CloseHandle(hPipe);
        }

        /*
        ������������ � ������ ������������� ������, ���������� � ������� ������� HeapAlloc ��� ���������� ������������, ������� �������������� � ������� ������� HeapFree, �������� ������� ������ � ����� ���������� winbase.h ��������� �������:

        BOOL HeapFree(
        HANDLE hHeap,
        DWORD dwFlags,
        LPVOID lpMem
        );

        ��� hHeap - ���������� ����, ������� � ������� ���������� � ������� ������� GetProcessHeap,
        ������� ���������� ���������� ����, ���������� �� �������� (�������� ������� ������ � �����
        ���������� winbase.h). �������� dwFlags ����� ��������� �������� HEAP_NO_SERIALIZE
        (������������� ��������� �������� 1). ��� ��������� ��������, ��� �� ����� �������������
        ����������� ������� ��� ������� ���������� ������� ���������� ������. �������� lpMem
        ������ �����, ������� � �������� ����� ����������� ������. � ������ �������� ����������
        ������� ���������� �� 0, � ����� - 0.
        */

        if (pSD)
            HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pSD);
    }

    void setState(int state)
    {
        PipeState = state;
    }

    //------------------------------------------------------------------
    /*
    ��������� ������������ ����� ������, ������� ������� ��������� ������������ ������ �
    ��������� ������ (PipeName), ��������� ���������� ���������� � ���� hPipe. ��������
    BufSize ������ ������ ������� �����-������, �������� DefWaitTime ������ ����� ��������
    ���������� ������� � ������� �� ���������, �������� ByteMode ��� ����� �������� true ������
    �������� �����, � � ��������� ������ - ����� ��������� ��� ������������ ������������
    ������. ����� ����� ������� ��������� ��������� ��������� OVERLAPPED � ������������
    ������� (�������� hEvent), ������� ������ ���� �������. ����� �������� ������������
    ������ ���������� ������ ������������ �������� ����������� ������� � ������
    (����� WaitClient, ������� �������� ���� ��������� ������)
    */

    DWORD CreatePipeAndWaitClient(wchar_t *PipeName, HANDLE hEvent, bool ByteMode = true, DWORD BufSize = DEF_BUF_SIZE, DWORD DefWaitTime = DEF_WAIT_TIME)
    {
        if (hEvent)
        {
            ClearOVERL();
            Overl.hEvent = hEvent;

            /*
            ��������� ������ ��� ���������� ������������ � ������� ������� HeapAlloc, ��������
            ������� ������ � ����� ���������� winbase.h ��������� �������:

            LPVOID HeapAlloc(
            HANDLE hHeap,
            DWORD dwFlags,
            SIZE_T dwBytes
            );

            ��� hHeap - ���������� ����, ������� � ������� ���������� � ������� ������� GetProcessHeap.
            �������� dwFlags ��������� � ������� ����������� �������� ��������� ��������� ���������
            ������. � �������� ����� ���������, ����� 0, ����� ���������� ��������� ����������� ��������:
            HEAP_GENERATE_EXCEPTIONS (�������� �������� 4) - ��������, ��� � ������ ���������� ����������
            ����� ������������� ��������������� ���������� (���� ���� ���� �� ������, �� � ���������
            �������� ������� ������ NULL);
            HEAP_NO_SERIALIZE (�������� �������� 1) - �������� ��. ����;
            HEAP_ZERO_MEMORY (�������� �������� 8) - ���������� ������ ����� ��������� ������.
            �������� dwBytes ������ ������ ���������� ������ � ������. � ������ �������� ����������
            ������� ������ ��������� �� ���������� �������� ������.
            */

            if (!pSD)
                pSD = HeapAlloc(GetProcessHeap(), HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, SECURITY_DESCRIPTOR_MIN_LENGTH);

            if (pSD && InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION) && SetSecurityDescriptorDacl(pSD, TRUE, NULL, FALSE))
            {

                /*
                ���� ������������� � ���������� ������� DACL ����������� ������������ ������ �������, ��
                ������������ �������� � ��������������� ������������� ���������� ������ SECURITY_ATTRIBUTES,
                ������� ����� ������������ ��� ������ ������� CreateNamedPipe
                */

                SECURITY_ATTRIBUTES sa = { sizeof(sa), pSD, true };

                hPipe = CreateNamedPipe(PipeName,
                    PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                    ByteMode ? PIPE_TYPE_BYTE | PIPE_READMODE_BYTE : PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
                    PIPE_UNLIMITED_INSTANCES,
                    BufSize,
                    BufSize,
                    DefWaitTime,
                    &sa
                );

                PipeState = WaitClient();
            }
        }

        return PipeState;
    }

    //------------------------------------------------------------------

    /*
    ��������� ������������ ����� ������, ������� � ������, ����� ������� �������� ����������
    ������ (����������� ������� IsOpen) ������������ ����������� �������� ����������� � ������
    � �������� ��������������� ������� ���� ��������� ������
    */


    DWORD WaitClient()
    {
        if (IsOpen())
        {
            // �������� � ������ ���
            PipeCurOperState = PIPE_NO_OPERATION;
            ClearOVERL();

            // ����������� �������� ����������� ������� � ������
            if (ConnectNamedPipe(hPipe, &Overl) == FALSE)
            {
                switch (GetLastError())
                {
                    // ����������� �������� ����������� ������� � ������ � ����������� ������
                case ERROR_IO_PENDING:		fPendingIOComplete = false;
                    return PipeState;

                    // ������ ����������� � ������																
                case ERROR_PIPE_CONNECTED:
                    // ������� ������� � ��������� ���������
                    SetEvent(Overl.hEvent);
                    PipeState = PIPE_CONNECTED;
                    CanCloseFlag = false;
                    fPendingIOComplete = true;
                    return PipeState;

                }
            }
            /*
            �������� ����������� ������� � ������ � ����������� ������ ��������� ��������
            */
            CanCloseFlag = false;
            fPendingIOComplete = true;
        }
        PipeState = PIPE_ERROR;

        return PipeState;
    }

    //------------------------------------------------------------------

    /*
    ��������� ������������ ����� ������, ������� � ������, ����� ������� �������� ����������
    ������ (����������� ������� IsOpen) ������������ ���������� ������� �� ������
    (����������� �� ������� ������� � ������, ����� ������ �������� ������������� ������).
    ��� ���� � ����������� �� ���������� ���������� �������� ���������� ���� ��������� ������
    */

    DWORD DisconnectClient()
    {
        if (IsOpen() && DisconnectNamedPipe(hPipe))
        {
            PipeState = PIPE_NOT_CONNECTED;
            return WaitClient();
        }
        else
        {
            fPendingIOComplete = true;
            return (PipeState = PIPE_ERROR);
        }
    }

    //------------------------------------------------------------------

    /*
    ��������� ������������ ����� ������, ������� � ������, ����� ������� �������� ����������
    ������ (����������� ������� IsOpen) ������������ ������ ����������� �������� ������ ������
    (������ ���� ��� ������ ����� �������� � �������� Message)
    */

    bool ReadMessage(std::basic_string<T> &Message)
    {
        DWORD NBytesRead;

        if (IsOpen())
        {

            bool fOverlapped = FALSE;

            if (!ReadFile(hPipe, &Message.at(0), sizeof(Message), &NBytesRead, &Overl))
                //BUG: out of range. It happens when server 
            {
                if (GetLastError() != ERROR_IO_PENDING)
                {
                    //��������� �����-�� ������ ��� ������ �� �����
                    if (GetLastError() == CLIENT_DISCONNECT)
                    {
                        std::cout << "Client disconnected! \n";
                        DisconnectClient();
                        WaitClient();
                        PipeCurOperState = PIPE_LOST_CONNECT;
                        return true;
                    }
                    else
                    {
                        std::cout << "Some error happened" << std::endl;
                        PipeCurOperState = PIPE_LOST_CONNECT;
                        return true;
                    }
                    
                    CanCloseFlag = false;
                    fPendingIOComplete = true;
                    PipeCurOperState = PIPE_NO_OPERATION;
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
            }

            if (fOverlapped)
            {
                // Wait for the operation to complete before continuing.
                // You could do some background work if you wanted to.
                if (GetOverlappedResult(hPipe, &Overl, &NBytesRead, TRUE))
                {
                    //���� ������ ��������� �������
                    std::cout << "Testing Message. Receiving data from client. \n";
                    Message[NBytesRead] = '\0'; //making end of string for parser
                    CanCloseFlag = true;
                    fPendingIOComplete = true;
                    PipeCurOperState = PIPE_JUST_CONNECTED;

                    return true;
                }
                else
                {
                    //�������� ���� ��������� ����������, �� ���-�� ����� �� ���
                    CancelIo(hPipe);
                    std::cout << "error reading file \n";

                }
            }
            else
            {
                //���� ������ ��������� �������
                Message[NBytesRead] = '\0';
                CanCloseFlag = true;
                fPendingIOComplete = true;
                PipeCurOperState = PIPE_JUST_CONNECTED;
            }
        }


        return false;
    }

    //------------------------------------------------------------------

    void WriteResponse(bool Message)
    {
        if (IsOpen())
        {
            DWORD NBWr;

            if (!WriteFile(hPipe, &Message, 1, &NBWr, NULL))
            {

                std::cout << "������ ������ � ����� " << GetLastError();
            }
        }
    }

    //------------------------------------------------------------------

    /*
    ��������� ������������ ����� ������, ������� � ������, ����� ������� �������� ����������
    ������ (����������� ������� IsOpen) ������������ �������� ���������� ����������� ��������
    � ��������� ���������� �������� ����������
    */

    bool GetPendingResult(DWORD &NBytesRead)
    {

        /*
        ���� ����������� �������� ��������� �������, �� ��������� ����� ��������� ������ �
        ��������� ������� �������� � ������ � ������ �������������
        */

        if (IsOpen() && GetOverlappedResult(hPipe, &Overl, &NBytesRead, FALSE) == TRUE)
        {
            if (PipeState == PIPE_NOT_CONNECTED)
            {
                PipeState = PIPE_CONNECTED;
                PipeCurOperState = PIPE_JUST_CONNECTED;
            }
            else
            {
                CanCloseFlag = true;
            }

            fPendingIOComplete = true;
            return true;
        }
        else
        {
            /*
            � ��������� ������ �������� ��������� ������ � ��������� ���� ��� ���������
            */
            CheckError();
        }

        return false;
    }

    //------------------------------------------------------------------

    /*
    ��������� ������������ ����� ������, ������� ��������� ��������� �������� ����������
    ������������ ������
    */

    bool IsOpen()
    {
        return hPipe != INVALID_HANDLE_VALUE;
    }

    //------------------------------------------------------------------

    /*
    ��������� ������������ ����� ������, ������� ���������� �������� ���� ���������
    ���������� ������
    */

    DWORD GetState()
    {
        return PipeState;
    }

    //------------------------------------------------------------------

    /*
    ��������� ������������ ����� ������, ������� ���������� �������� ���� ���������
    ������� �������� ��� ����� ���������� ������
    */

    DWORD GetOperState()
    {
        return PipeCurOperState;
    }

    //------------------------------------------------------------------
    /*
    ��������� ������������ ����� ������, ������� ���������� �������� ��������
    ���������� ����������� �������� ��� ����� ���������� ������
    */

    bool GetIOComplete()
    {
        return fPendingIOComplete;
    }

    //------------------------------------------------------------------


    bool CanClose()
    {
        return CanCloseFlag;
    }

    //------------------------------------------------------------------

    bool readFromDB(std::ifstream &file)
    {
        if (file >> maxLenLog >> maxLenPass)
        {
            T *tmpLogin = new T[maxLenLog];
            T *tmpPassword = new T[maxLenPass];
            while (file >> tmpLogin >> tmpPassword)
            {
                db_users.push_back({ tmpLogin, tmpPassword });
            }
            delete[] tmpLogin;
            delete[] tmpPassword;
            return true;
        }

        return false;
    }

    //------------------------------------------------------------------

    bool checkUser(const std::vector<User<T>> &vec)
    {
        if (!vec.empty())
        {
            for (size_t i = 0; i < db_users.size() && i < vec.size(); ++i)
            {
                if (db_users[i].login == vec[i].login && db_users[i].password == vec[i].password)
                {
                    return true;
                }
            }
        }
        return false;
    }
};