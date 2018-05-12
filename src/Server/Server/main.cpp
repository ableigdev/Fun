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
        Вывод в лог файл
        сколько времени потрачено на "отбитие атаки"

        в клиенте:
        вывод сколько возможных паролей
        сколько времени было затрачено на подбор пароля
        
    */

    /*
    Описание переменных:
    hEvents - массив из MAX_PIPE_INST элементов, содержащий дескрипторы событий;
    PipeInfo - массив из MAX_PIPE_INST элементов, содержащий экземпляры классов PerPipeStruct для каждого экземпляра канала;
    Pipes - массив из MAX_PIPE_INST элементов, содержащий экземпляры классов CPipeServer для каждого экземпляра канала;
    FName - строка, в которую будет введено пользователем имя файла;
    answer - переменная, в которую после соответствующего запроса будет введен ответ пользователя про необходимость продолжения работу сервера;
    file - экземпляр класса ofstream, с помощью которого будет осуществляться запись в файл;
    PipeNumber - номер экземпляра канала, для которого завершилась асинхронная операция;
    NBytesRead - количество прочитанных из канала байт;
    Message - прочитанное из канала сообщение (независимо от режима работы канала байтного или режима сообщений);
    PipesConnect - автоматически изменяемое количество одновременно подключенных клиентов (при подключении очередного клиента увеличивается на 1, при отключении - уменьшается на 1).
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
    std::ofstream fout ("server_log.txt", std::ios_base::app); // вывод в лог
    SYSTEMTIME st = {};
    


    SetConsoleOutputCP(1251);
    std::cout << "Введите имя файла с логинами и паролями: ";
    
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
		std::cout << "\nФайл с паролями не выбран!" << std::endl;
		std::cout << "Выберите файл." << std::endl;
	}

	std::wcout << FName << std::endl;

    file.open(FName);

    if (!file)
    {
        std::cout << "Ошибка чтения файла с именем " << FName << "!" << std::endl;
    }
    else if(!fout)
    {
        std::cout << "Ошибка log файла!" << std::endl;
    }
    else
    {
        std::cout << "\nВыберите режим работы сервера:";
        std::cout << "\n0 - Обычный;\n1 - Противодействие взлому.\n";
        std::cin.clear();
        std::cin >> serverMode;

        int attempt_counter[MAX_PIPE_INST] = { 0 };
        int unblock_time[MAX_PIPE_INST] = { 0 };

        for (int i = 0; i < MAX_PIPE_INST; i++)
        {
            /*
            Получение дескрипторов создаваемых событий и создание экземпляров именованных каналов и
            перевод их в режим ожидания подключения клиента
            */

            hEvents[i] = CreateEvent(NULL, TRUE, TRUE, NULL);
            Pipes[i].CreatePipeAndWaitClient(PIPE_NAME, hEvents[i]);

            if (Pipes[i].GetState() == PIPE_ERROR)
            {
                /*
                Если произошла ошибка при создании экземпляра именованного канала, то освобождение
                всех выделенных в программе ресурсов и завершение приложения
                */

                file.close();
                delete[]FName;

                for (i--; i >= 0; i--)
                    CloseHandle(hEvents[i]);

                std::wcout << "Ошибка создания экземпляров именованного канала " << PIPE_NAME << std::endl;

                return 0;
            }
        }

        fout    << "\n\n --------------------------------------------------------------------------------------------------------------------------\n" 
                << getCurDateStr(st) << "\t Запущен сервер. Настройки: \n\t Имя файла с паролями: " << FName << "\n\t Режим работы: " << serverMode << "\n";

        /*
        Бесконечный цикл, составляющий логическое ядро сервера
        */
        std::cout << "Ожидание подключения клиентов..." << std::endl;
		
		int counter = 0;
        do
        {
            
            Message.resize(100);
            
            /*
            Ожидания перехода в свободное состояние события, связанного с каким-то из каналов,
            что свидетельствует о завершении асинхронной операции, и получения номера этого канала.
            При этом функция WaitForMultipleObjects состояние события с ручным сбросом
            АВТОМАТИЧЕСКИ НЕ ИЗМЕНЯЕТ.
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
                Если получен правильный номер, то проверяется, была ли запущена асинхронная операция, и,
                если она была запущена, то проверяется состояние завершенной асинхронной операции методом
                GetPendingResult и выполняются соответствующие изменения полей состояния экземпляра канала
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
					bool hasData = false; // Флаг для проверки наличия данных в канале (чтобы сервер в случае неправильного логина или пароля гарантировано 3 раза считал данные)
                    int readTime = clock() + 1000;

                    /*
                    Если установлен признак завершения асинхронной операции для указанного экземпляра канала,
                    то проверка поля его состояния
                    */
                    switch (Pipes[PipeNumber].GetState())
                    {
                        /*
                        В случае, когда при работе с каналом происходит непредвиденная ошибка
                        */
                    case PIPE_ERROR:
                        std::cout << "Ошибка при работе с каналом! Производится принудительное отсоединение клиента (код ошибки: "
                            << GetLastError() << ")!" << std::endl;
                        fout << getCurDateStr(st) << "\tОшибка при работе с каналом! Производится принудительное отсоединение клиента (код ошибки: "
                            << GetLastError() << ")!" << std::endl;

                        /*
                        Если клиент подключен, то проверка того, подключился ли он только что (соответствующее
                        значение в поле состояния операции в канале), и, если это так, то увеличение количества
                        подключенных клиентов на 1.
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
						// Если логин/пароль неправильный и счетчик поптыок меньше макс. кол-ву попыток, а также клиент не отключился раньше времени
                        while (!resultCheckUser && (attempt_counter[PipeNumber] * serverMode) < MAX_COUNTER_ATTEMPT && Pipes[PipeNumber].GetState() != PIPE_LOST_CONNECT)
                        {
                            //clock() - функция получения текущего времени
                            if (clock() >= unblock_time[PipeNumber])
                            {
                                
                                
                                if (Pipes[PipeNumber].ReadMessage(Message)) // если данные есть в канале
                                {
                                    readTime = clock() + 1000;

                                    if (Message.size() == 1 && (Message == "C" || Message == "c"))
                                    {
                                        Pipes[PipeNumber].setState(PIPE_LOST_CONNECT);
                                        break;
                                    }
                                    hasData = true;
                                    PipeInfo[PipeNumber].ReadVal(Message); // записываем их в вектор
                                    tempData = PipeInfo[PipeNumber].getData(); // получаем элемент вектора

                                    fout << getCurDateStr(st) << "\tTesting Message. Write Response" << std::endl;
                                    fout << getCurDateStr(st) << "\tAuth attempt (" << tempData[0].login << "/" << tempData[0].password << ")\n";


                                    resultCheckUser = Pipes[PipeNumber].checkUser(tempData); // сверяем логин и пароль с базой
                                    Pipes[PipeNumber].WriteResponse(resultCheckUser); // отправляем результат проверки

                                    Message.clear(); // Очищаем буфер

                                    //for debug----------------------------------------
                                    fout << getCurDateStr(st) << "\tTesting Message. Client auth status: ";
                                    if (resultCheckUser)
                                    {
                                        //std::cout << "TRUE";
                                        fout << "TRUE\n";
                                    }
                                    else
                                    {
                                        //std::cout << "FALSE" << " Attempt №" << attempt_counter[PipeNumber];
                                        fout << "FALSE" << " Attempt №" << attempt_counter[PipeNumber] << "\n";
                                        Message.resize(100); // Расширяем размер буфера до исходного
                                    }
                                    //--------------------------------------------------

                                    PipeInfo[PipeNumber].ClearData();
                                    tempData.clear();
                                    ++attempt_counter[PipeNumber];

                                    //latency <<= 5;
                                    //fine переводится как штраф
                                    int fine = (100 * attempt_counter[PipeNumber]) * serverMode;
                                    fout << getCurDateStr(st) << "\tTime to response: " << fine << "мс" << std::endl;
                                    unblock_time[PipeNumber] = clock() + fine;
                                    break;
                                }
                                else if (hasData) // Если считали данные, то продолжаем ожидать следующую порцию данных от клиента 
                                {
                                    if (clock() > readTime)
                                    {
                                        Pipes[PipeNumber].WriteResponse(resultCheckUser);
                                        std::cout << "\nКлиент не отвечает, повторный запрос.\n";
                                        fout << getCurDateStr(st) << "\tКлиент не отвечает, повторный запрос.\n";
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
                        Отключение клиента. В этом случае происходит вывод данных, прочитанных из канала в файл и
                        их очистка в структуре данных, уменьшения количество подключенных клиентов на 1,
                        отключение клиента со стороны сервера и запуск ожидания подключения нового клиента
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
                            std::cout << "В канал не было передано никаких данных со стороны клиента. Повторить попытку чтения данныхY или y - да / любая другая клавиша - нет)?" << std::endl;
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
				Если нет подключенных клиентов, то запрос о необходимости продолжения работы и выполнение
				соответствующих действий
				*/
				if (PipesConnect == 0)
				{
					std::cout << "Все клиенты отключены! Продолжить работу (Y или y - да / любая другая клавиша - нет)? ";
					fout << getCurDateStr(st) << "\tВсе клиенты отключены!\n";
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
        Освобождение выделенных в программе ресурсов
        */

		for (int i = 0; i < MAX_PIPE_INST; ++i)
		{
			CloseHandle(hEvents[i]);
		}
        file.close();
        fout << getCurDateStr(st) << "\tВыключение сервера.";
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