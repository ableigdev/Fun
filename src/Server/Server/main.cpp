#include <iostream>
#include <fstream>
#include <Windows.h>

#include "PipeServer.h"
#include "PerPipeStruct.h"

#define MAX_PIPE_INST	3
#define PIPE_NAME		L"\\\\.\\pipe\\pipe_example"
#define MAX_COUNTER_ATTEMPT 3
#define EXHAUSTED_ATTEMPTS -1

int main()
{
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
    std::cout << "Введите имя файла с логинами и паролями: ";
    std::cin >> FName;

    file.open(FName);

    if (!file)
        std::cout << "Ошибка чтения файла с именем " << FName << "!" << std::endl;
    else
    {

        std::cout << "Выберите режим работы сервера:";
        std::cout << "\n0 - Обычный;\n1 - Противодействие взлому.\n";
        std::cin.clear();
        std::cin >> serverMode;



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

        
        /*
        Бесконечный цикл, составляющий логическое ядро сервера
        */

        do
        {
            std::cout << "Ожидание подключения клиентов..." << std::endl;
            Message.resize(100);

            /*
            Ожидания перехода в свободное состояние события, связанного с каким-то из каналов,
            что свидетельствует о завершении асинхронной операции, и получения номера этого канала.
            При этом функция WaitForMultipleObjects состояние события с ручным сбросом
            АВТОМАТИЧЕСКИ НЕ ИЗМЕНЯЕТ.
            */

            PipeNumber = WaitForMultipleObjects(MAX_PIPE_INST, hEvents, FALSE, INFINITE) - WAIT_OBJECT_0;
         
            if (!file.is_open())
            {
                file.open(FName);
            }

            Pipes[PipeNumber].readFromDB(file);
            file.close();

            if (PipeNumber < MAX_PIPE_INST)
            {

                /*
                Если получен правильный номер, то проверяется, была ли запущена асинхронная операция, и,
                если она была запущена, то проверяется состояние завершенной асинхронной операции методом
                GetPendingResult и выполняются соответствующие изменения полей состояния экземпляра канала
                */

                if (!Pipes[PipeNumber].GetIOComplete())
                    Pipes[PipeNumber].GetPendingResult(NBytesRead);

                if (Pipes[PipeNumber].GetIOComplete())
                {

                    bool resultCheckUser = false;
                    long long latency = (serverMode) ? 1 : 0;
                    int i = 0;
                    auto tempData = PipeInfo[PipeNumber].getData();
					bool hasData = false; // Флаг для проверки наличия данных в канале (чтобы сервер в случае неправильного логина или пароля гарантировано 3 раза считал данные)

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

                        /*
                        Если клиент подключен, то проверка того, подключился ли он только что (соответствующее
                        значение в поле состояния операции в канале), и, если это так, то увеличение количества
                        подключенных клиентов на 1.
                        */
                    case PIPE_CONNECTED:
                        if (Pipes[PipeNumber].GetOperState() == PIPE_JUST_CONNECTED)
                        {
                            std::cout << "Testing Message. Just connected" << std::endl;
                            PipesConnect++;
                        }

                        resultCheckUser = false;
                        latency = (serverMode) ? 1 : 0;
                        i = 0;
                        tempData = PipeInfo[PipeNumber].getData();

						// Если логин/пароль неправильный и счетчик поптыок меньше макс. кол-ву попыток, а также клиент не отключился раньше времени
                        while (!resultCheckUser && i < MAX_COUNTER_ATTEMPT && Pipes[PipeNumber].GetState() != PIPE_LOST_CONNECT)
                        {
                            Sleep(latency);

                            if (Pipes[PipeNumber].ReadMessage(Message)) // если данные есть в канале
                            {
								if (Message == "C" || Message == "c")
								{
									Pipes[PipeNumber].setState(PIPE_LOST_CONNECT);
									break;
								}
								hasData = true;
								PipeInfo[PipeNumber].ReadVal(Message); // записываем их в вектор
								tempData = PipeInfo[PipeNumber].getData(); // получаем элемент вектора
								std::cout << "Testing Message. Write Response" << std::endl;
								resultCheckUser = Pipes[PipeNumber].checkUser(tempData); // сверяем логин и пароль с базой
								Pipes[PipeNumber].WriteResponse(resultCheckUser); // отправляем результат проверки

								Message.clear(); // Очищаем буфер

								//for debug----------------------------------------
								std::cout << "Testing Message. Client auth status: ";
								if (resultCheckUser)
								{
									std::cout << "TRUE";
								}
								else
								{
									std::cout << "FALSE";
									Message.resize(100); // Расширяем размер буфера до исходного
								}
								std::cout << std::endl;
								//--------------------------------------------------

								PipeInfo[PipeNumber].ClearData();
								tempData.clear();
								++i;
								latency <<= 5;
                            }	
							else if (hasData) // Если считали данные, то продолжаем ожидать следующую порцию данных от клиента 
							{
								++i;
								hasData = false;
							}
                        }

						if (i >= MAX_COUNTER_ATTEMPT)
						{
							Pipes[PipeNumber].WriteResponse(EXHAUSTED_ATTEMPTS);
						}
                        Pipes[PipeNumber].setState(PIPE_LOST_CONNECT);
                        break;

                        /*
                        Отключение клиента. В этом случае происходит вывод данных, прочитанных из канала в файл и
                        их очистка в структуре данных, уменьшения количество подключенных клиентов на 1,
                        отключение клиента со стороны сервера и запуск ожидания подключения нового клиента
                        */

                    case PIPE_LOST_CONNECT:

                        if (PipesConnect > 0)
                        {
                            PipesConnect--;
                        }

                        //Pipes[PipeNumber].WriteResponse(resultCheckUser);
                        std::cout << "Testing Message. Client disconnected.\n";
                        Pipes[PipeNumber].DisconnectClient();

                        //Pipes[PipeNumber].WaitClient();
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
            }

            /*
            Если нет подключенных клиентов, то запрос о необходимости продолжения работы и выполнение
            соответствующих действий
            */

            if (PipesConnect == 0)
            {
                std::cout << "Все клиенты отключены! Продолжить работу (Y или y - да / любая другая клавиша - нет)? ";
                std::cin >> answer;
                if (answer != 'Y' && answer != 'y')
                {
                    break;
                }
                vec.clear();
                Message.clear(); // makes size of string == 0


                //std::cout << "Ожидание подключения клиентов..." << std::endl;
            }

        } while (1);

        /*
        Освобождение выделенных в программе ресурсов
        */

        for (int i = 0; i < MAX_PIPE_INST; i++)
            CloseHandle(hEvents[i]);

        file.close();
    }

    delete[]FName;

    return 0;
}