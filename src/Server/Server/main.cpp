#include <iostream>
#include <fstream>
#include <Windows.h>
#include <sstream>


#include "PipeServer.h"
#include "PerPipeStruct.h"

#include <ctime>

#define MAX_PIPE_INST	3
#define PIPE_NAME		L"\\\\.\\pipe\\pipe_example"
#define MAX_COUNTER_ATTEMPT 20000
#define EXHAUSTED_ATTEMPTS -1

std::string getCurDateStr(SYSTEMTIME st);

int main()
{
    /*
        ����� � ��� ����
        ������� ������� ��������� �� "������� �����"

        � �������:
        ����� ������� ��������� �������
        ������� ������� ���� ��������� �� ������ ������
        
    */

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
    TCHAR *FName = new TCHAR[MAX_PATH];
    char answer;
    std::ifstream file;
    DWORD PipeNumber, NBytesRead;
    std::basic_string<char> Message{};
    int PipesConnect = 0;
    bool serverMode = false;
    std::vector<User<char>> vec;
    std::ofstream fout ("server_log.txt", std::ios_base::app); // ����� � ���
    SYSTEMTIME st = {};
    


    SetConsoleOutputCP(1251);
    std::cout << "������� ��� ����� � �������� � ��������: ";
    
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFilter = TEXT("All\0*\0\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = FName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(CHAR)*MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	while (!GetOpenFileName(&ofn))
	{
		std::cout << "\n���� � �������� �� ������!" << std::endl;
		std::cout << "�������� ����." << std::endl;
	}

	std::wcout << FName << std::endl;

    file.open(FName);

    if (!file)
    {
        std::cout << "������ ������ ����� � ������ " << FName << "!" << std::endl;
    }
    else if(!fout)
    {
        std::cout << "������ log �����!" << std::endl;
    }
    else
    {
        std::cout << "\n�������� ����� ������ �������:";
        std::cout << "\n0 - �������;\n1 - ��������������� ������.\n";
        std::cin.clear();
        std::cin >> serverMode;

        int attempt_counter[MAX_PIPE_INST] = { 0 };
        int unblock_time[MAX_PIPE_INST] = { 0 };

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

        fout    << "\n\n --------------------------------------------------------------------------------------------------------------------------\n" 
                << getCurDateStr(st) << "\t ������� ������. ���������: \n\t ��� ����� � ��������: " << FName << "\n\t ����� ������: " << serverMode << "\n";

        /*
        ����������� ����, ������������ ���������� ���� �������
        */
        std::cout << "�������� ����������� ��������..." << std::endl;
		
		int counter = 0;
        do
        {
            
            Message.resize(100);
            
            /*
            �������� �������� � ��������� ��������� �������, ���������� � �����-�� �� �������,
            ��� ��������������� � ���������� ����������� ��������, � ��������� ������ ����� ������.
            ��� ���� ������� WaitForMultipleObjects ��������� ������� � ������ �������
            ������������� �� ��������.
            */
			
			PipeNumber = WaitForSingleObject(hEvents[counter], 15);

            if (PipeNumber < MAX_PIPE_INST)
            {
				if (PipeNumber != counter)
				{
					PipeNumber = counter;
				}

				if (!file.is_open())
				{
					file.open(FName);
				}

				Pipes[PipeNumber].readFromDB(file);
				file.close();

                /*
                ���� ������� ���������� �����, �� �����������, ���� �� �������� ����������� ��������, �,
                ���� ��� ���� ��������, �� ����������� ��������� ����������� ����������� �������� �������
                GetPendingResult � ����������� ��������������� ��������� ����� ��������� ���������� ������
                */

				if (!Pipes[PipeNumber].GetIOComplete())
				{
					Pipes[PipeNumber].GetPendingResult(NBytesRead);
				}

                if (Pipes[PipeNumber].GetIOComplete())
                {
                    bool resultCheckUser = false;
                    long long latency = (serverMode) ? 1 : 0;
                    int i = 0;
                    auto tempData = PipeInfo[PipeNumber].getData();
					bool hasData = false; // ���� ��� �������� ������� ������ � ������ (����� ������ � ������ ������������� ������ ��� ������ ������������� 3 ���� ������ ������)
                    int readTime = clock() + 1000;

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
                        fout << getCurDateStr(st) << "\t������ ��� ������ � �������! ������������ �������������� ������������ ������� (��� ������: "
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
                            fout << getCurDateStr(st) << "\tTesting Message. Just connected" << std::endl;
                            ++PipesConnect;
                        }

                        resultCheckUser = false;
                        latency = (serverMode) ? 1 : 0;
                        
                        tempData = PipeInfo[PipeNumber].getData();
                        
                        readTime = clock() + 1000;
						// ���� �����/������ ������������ � ������� ������� ������ ����. ���-�� �������, � ����� ������ �� ���������� ������ �������
                        while (!resultCheckUser && (attempt_counter[PipeNumber] * serverMode) < MAX_COUNTER_ATTEMPT && Pipes[PipeNumber].GetState() != PIPE_LOST_CONNECT)
                        {
                            //clock() - ������� ��������� �������� �������
                            if (clock() >= unblock_time[PipeNumber])
                            {
                                
                                
                                if (Pipes[PipeNumber].ReadMessage(Message)) // ���� ������ ���� � ������
                                {
                                    readTime = clock() + 1000;

                                    if (Message.size() == 1 && (Message == "C" || Message == "c"))
                                    {
                                        Pipes[PipeNumber].setState(PIPE_LOST_CONNECT);
                                        break;
                                    }
                                    hasData = true;
                                    PipeInfo[PipeNumber].ReadVal(Message); // ���������� �� � ������
                                    tempData = PipeInfo[PipeNumber].getData(); // �������� ������� �������

                                    fout << getCurDateStr(st) << "\tTesting Message. Write Response" << std::endl;
                                    fout << getCurDateStr(st) << "\tAuth attempt (" << tempData[0].login << "/" << tempData[0].password << ")\n";


                                    resultCheckUser = Pipes[PipeNumber].checkUser(tempData); // ������� ����� � ������ � �����
                                    Pipes[PipeNumber].WriteResponse(resultCheckUser); // ���������� ��������� ��������

                                    Message.clear(); // ������� �����

                                    //for debug----------------------------------------
                                    fout << getCurDateStr(st) << "\tTesting Message. Client auth status: ";
                                    if (resultCheckUser)
                                    {
                                        //std::cout << "TRUE";
                                        fout << "TRUE\n";
                                    }
                                    else
                                    {
                                        //std::cout << "FALSE" << " Attempt �" << attempt_counter[PipeNumber];
                                        fout << "FALSE" << " Attempt �" << attempt_counter[PipeNumber] << "\n";
                                        Message.resize(100); // ��������� ������ ������ �� ���������
                                    }
                                    //--------------------------------------------------

                                    PipeInfo[PipeNumber].ClearData();
                                    tempData.clear();
                                    ++attempt_counter[PipeNumber];

                                    //latency <<= 5;
                                    //fine ����������� ��� �����
                                    int fine = (100 * attempt_counter[PipeNumber]) * serverMode;
                                    fout << getCurDateStr(st) << "\tTime to response: " << fine << "��" << std::endl;
                                    unblock_time[PipeNumber] = clock() + fine;
                                    break;
                                }
                                else if (hasData) // ���� ������� ������, �� ���������� ������� ��������� ������ ������ �� ������� 
                                {
                                    if (clock() > readTime)
                                    {
                                        Pipes[PipeNumber].WriteResponse(resultCheckUser);
                                        std::cout << "\n������ �� ��������, ��������� ������.\n";
                                        fout << getCurDateStr(st) << "\t������ �� ��������, ��������� ������.\n";
                                    }
                                    ++attempt_counter[PipeNumber];
                                    hasData = false;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }

						if ((attempt_counter[PipeNumber] * serverMode) >= MAX_COUNTER_ATTEMPT)
						{
							Pipes[PipeNumber].WriteResponse(EXHAUSTED_ATTEMPTS); 
                            Pipes[PipeNumber].setState(PIPE_LOST_CONNECT);
						}
                        
                        break;

                        /*
                        ���������� �������. � ���� ������ ���������� ����� ������, ����������� �� ������ � ���� �
                        �� ������� � ��������� ������, ���������� ���������� ������������ �������� �� 1,
                        ���������� ������� �� ������� ������� � ������ �������� ����������� ������ �������
                        */

                    case PIPE_LOST_CONNECT:

                        if (PipesConnect > 0)
                        {
                            --PipesConnect;
                        }

                        std::cout << "Testing Message. Client disconnected.\n";
                        fout << getCurDateStr(st) << "\tTesting Message. Client disconnected.\n";
                        Pipes[PipeNumber].DisconnectClient();

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

				/*
				���� ��� ������������ ��������, �� ������ � ������������� ����������� ������ � ����������
				��������������� ��������
				*/
				if (PipesConnect == 0)
				{
					std::cout << "��� ������� ���������! ���������� ������ (Y ��� y - �� / ����� ������ ������� - ���)? ";
					fout << getCurDateStr(st) << "\t��� ������� ���������!\n";
					memset(attempt_counter, 0, sizeof(attempt_counter));

					std::cin >> answer;
					if (answer != 'Y' && answer != 'y')
					{
						break;
					}
					vec.clear();
					Message.clear(); // makes size of string == 0
				}
            }

            

			if (counter < MAX_PIPE_INST - 1)
			{
				++counter;
			}
			else
			{
				counter = 0;
			}

        } while (1);

        /*
        ������������ ���������� � ��������� ��������
        */

		for (int i = 0; i < MAX_PIPE_INST; ++i)
		{
			CloseHandle(hEvents[i]);
		}
        file.close();
        fout << getCurDateStr(st) << "\t���������� �������.";
        fout.close();
    }
    

    delete[]FName;

    return 0;
}

std::string getCurDateStr(SYSTEMTIME st)
{
    GetLocalTime(&st);
    std::stringstream ss;
    //put arbitrary formatted data into the stream
    ss << st.wYear << "-" << st.wMonth << "-" << st.wDay << " " << st.wHour << ":" << st.wMinute << ":" << st.wSecond << "\t" ;
    //convert the stream buffer into a string
    std::string str = ss.str();
    return str;
}