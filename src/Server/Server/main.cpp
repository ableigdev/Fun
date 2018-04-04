#include <iostream>
#include <fstream>
#include <Windows.h>

#include "PipeServer.h"
#include "PerPipeStruct.h"

#define MAX_PIPE_INST	3
#define PIPE_NAME		L"\\\\.\\pipe\\pipe_example"
#define MAX_COUNTER_ATTEMPT 3

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
    char *FName = new char[MAX_PATH];
    char answer;
    std::ifstream file;
    DWORD PipeNumber, NBytesRead;
    std::basic_string<char> Message{};
    //Message.resize(100); // moved to loop 
    int PipesConnect = 0;
    bool serverMode = false;
    std::vector<User<char>> vec;


    SetConsoleOutputCP(1251);
    std::cout << "������� ��� ����� � �������� � ��������: ";
    std::cin >> FName;

    file.open(FName);

    if (!file)
        std::cout << "������ ������ ����� � ������ " << FName << "!" << std::endl;
    else
    {

        std::cout << "�������� ����� ������ �������:";
        std::cout << "\n0 - �������;\n1 - ��������������� ������.\n";
        std::cin.clear();
        std::cin >> serverMode;



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
            Message.resize(100);

            /*
            �������� �������� � ��������� ��������� �������, ���������� � �����-�� �� �������,
            ��� ��������������� � ���������� ����������� ��������, � ��������� ������ ����� ������.
            ��� ���� ������� WaitForMultipleObjects ��������� ������� � ������ �������
            ������������� �� ��������.
            */

            PipeNumber = WaitForMultipleObjects(MAX_PIPE_INST, hEvents, FALSE, INFINITE) - WAIT_OBJECT_0;
            bool flag = true;

            if (!file.is_open())
            {
                file.open(FName);
            }

            Pipes[PipeNumber].readFromDB(file);
            file.close();

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


                        do
                        {
                            flag = Pipes[PipeNumber].ReadMessage(Message);

                            if (Message[0] != '\0')
                            {
                                vec.push_back(PipeInfo[PipeNumber].parseString(Message));

                                PipeInfo[PipeNumber].ReadVal(Message);
                                /*
                                ���� ��������� ����������� �������� ������, �� �������� ��������� ��������
                                */

                                switch (Pipes[PipeNumber].GetOperState())
                                {
                                    /*
                                    ���� ��������� �� ��� ������ ���������, �� ������ ���������� ������
                                    */

                                case PIPE_READ_PART:
                                    Pipes[PipeNumber].ReadMessage(Message);
                                    std::cout << "Testing Message. Reading data part" << std::endl;
                                    break;

                                    /*
                                    ���� ������ ��������� ��������� �������, �� ��������� ����������� ��������
                                    */

                                case PIPE_READ_SUCCESS:
                                    PipeInfo[PipeNumber].ReadVal(Message);
                                    std::cout << "Testing Message. Reading data" << std::endl;
                                    break;

                                    /*
                                    ��������� ������ ������
                                    */
                                case PIPE_OPERATION_ERROR:
                                    std::cout << "������ ��� ������ ������ �� ������ (��� ������: "
                                        << GetLastError() << ")!" << std::endl;
                                    break;

                                }
                                break;
                            }
                            //break;
                        } while (!flag);

                        /*
                        ���������� �������. � ���� ������ ���������� ����� ������, ����������� �� ������ � ���� �
                        �� ������� � ��������� ������, ���������� ���������� ������������ �������� �� 1,
                        ���������� ������� �� ������� ������� � ������ �������� ����������� ������ �������
                        */

                    case PIPE_LOST_CONNECT:
                        bool resultCheckUser = false;
                        long long latency = (serverMode) ? 1 : 0;
                        int i = 0;
                        auto tempData = PipeInfo[PipeNumber].getData();
                        bool firstTimeFlag = true;


                        /*
                        std::cout << "Testing Message. Write Response" << std::endl;
                        auto tempData = PipeInfo[PipeNumber].getData(); // ��������� ����� � ������
                        bool resultCheckUser = Pipes[PipeNumber].checkUser(tempData); // ��������� �������� �����
                        int i = 1;
                        long long latency = (serverMode) ? 1 : 0;
                        Pipes[PipeNumber].WriteResponse(resultCheckUser);

                        //for debug----------------------------------------
                        std::cout << "Testing Message. Client auth status: ";
                        if (resultCheckUser) std::cout << "TRUE";
                        else std::cout << "FALSE";
                        std::cout << std::endl;
                        //--------------------------------------------------
                        */

                        // � ������ ���� �� ������� �� �������� ������ � ������ �������� �����, � �� ���������� ������ � ���� �� ����... ���������� ���. 
                        // 04.04 - ����� � ���� ����������, �� �������� ������� ����� ��������
                        while (!resultCheckUser && i < MAX_COUNTER_ATTEMPT)
                        {
                            Sleep(latency);

                            if (firstTimeFlag || Pipes[PipeNumber].ReadMessage(Message)) // ���� ������ ���� � ������
                            {
                                firstTimeFlag = false; // ������� �� ��������, �� � ��� ��� ��� �� �����
                                if (GetLastError() == CLIENT_DISCONNECT)
                                    break;

                                PipeInfo[PipeNumber].ReadVal(Message); // ���������� �� � ������
                                tempData = PipeInfo[PipeNumber].getData(); // �������� ������� �������
                                std::cout << "Testing Message. Write Response" << std::endl;
                                resultCheckUser = Pipes[PipeNumber].checkUser(tempData); // ������� ����� � ������ � �����
                                Pipes[PipeNumber].WriteResponse(resultCheckUser); // ���������� ��������� ��������

                                //for debug----------------------------------------
                                std::cout << "Testing Message. Client auth status: ";
                                if(resultCheckUser) std::cout << "TRUE";
                                else std::cout << "FALSE";
                                std::cout << std::endl;
                                //--------------------------------------------------

                                PipeInfo[PipeNumber].ClearData();
                                ++i;
                                latency <<= 5;
                            }
                        }


                        if (PipesConnect > 0)
                        {
                            PipesConnect--;
                        }

                        Pipes[PipeNumber].WriteResponse(resultCheckUser);
                        std::cout << "Testing Message. Client disconnected.";
                        Pipes[PipeNumber].DisconnectClient();

                        Pipes[PipeNumber].WaitClient();
                        if (Pipes[PipeNumber].CanClose() == false)
                        {
                            std::cout << "� ����� �� ���� �������� ������� ������ �� ������� �������. ��������� ������� ������ ������Y ��� y - �� / ����� ������ ������� - ���)?" << std::endl;
                            std::cin >> answer;
                            if (answer == 'Y' || answer == 'y')
                            {
                                continue;
                            }
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
                {
                    break;
                }
                vec.clear();
                Message.clear(); // makes size of string == 0


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