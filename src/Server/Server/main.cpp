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

        std::cout << "Ожидание подключения клиентов..." << std::endl;
        /*
        Бесконечный цикл, составляющий логическое ядро сервера
        */

        do
        {
            Message.resize(100);

            /*
            Ожидания перехода в свободное состояние события, связанного с каким-то из каналов,
            что свидетельствует о завершении асинхронной операции, и получения номера этого канала.
            При этом функция WaitForMultipleObjects состояние события с ручным сбросом
            АВТОМАТИЧЕСКИ НЕ ИЗМЕНЯЕТ.
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
                Если получен правильный номер, то проверяется, была ли запущена асинхронная операция, и,
                если она была запущена, то проверяется состояние завершенной асинхронной операции методом
                GetPendingResult и выполняются соответствующие изменения полей состояния экземпляра канала
                */

                if (!Pipes[PipeNumber].GetIOComplete())
                    Pipes[PipeNumber].GetPendingResult(NBytesRead);

                if (Pipes[PipeNumber].GetIOComplete())
                {
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


                        do
                        {
                            flag = Pipes[PipeNumber].ReadMessage(Message);

                            if (Message[0] != '\0')
                            {
                                vec.push_back(PipeInfo[PipeNumber].parseString(Message));

                                PipeInfo[PipeNumber].ReadVal(Message);
                                /*
                                Если завершена асинхронная операция чтения, то проверка состояния операции
                                */

                                switch (Pipes[PipeNumber].GetOperState())
                                {
                                    /*
                                    Если прочитаны не все данные сообщения, то запуск повторного чтения
                                    */

                                case PIPE_READ_PART:
                                    Pipes[PipeNumber].ReadMessage(Message);
                                    std::cout << "Testing Message. Reading data part" << std::endl;
                                    break;

                                    /*
                                    Если чтение сообщения завершено успешно, то обработка полученного значения
                                    */

                                case PIPE_READ_SUCCESS:
                                    PipeInfo[PipeNumber].ReadVal(Message);
                                    std::cout << "Testing Message. Reading data" << std::endl;
                                    break;

                                    /*
                                    Произошла ошибка чтения
                                    */
                                case PIPE_OPERATION_ERROR:
                                    std::cout << "Ошибка при чтении данных из канала (код ошибки: "
                                        << GetLastError() << ")!" << std::endl;
                                    break;

                                }
                                break;
                            }
                            //break;
                        } while (!flag);

                        /*
                        Отключение клиента. В этом случае происходит вывод данных, прочитанных из канала в файл и
                        их очистка в структуре данных, уменьшения количество подключенных клиентов на 1,
                        отключение клиента со стороны сервера и запуск ожидания подключения нового клиента
                        */

                    case PIPE_LOST_CONNECT:
                        bool resultCheckUser = false;
                        long long latency = (serverMode) ? 1 : 0;
                        int i = 0;
                        auto tempData = PipeInfo[PipeNumber].getData();
                        bool firstTimeFlag = true;


                        /*
                        std::cout << "Testing Message. Write Response" << std::endl;
                        auto tempData = PipeInfo[PipeNumber].getData(); // считанные логин и пароль
                        bool resultCheckUser = Pipes[PipeNumber].checkUser(tempData); // результат проверки юзера
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

                        // в идеале было бы сделать всё проверку сверху в первой итерации цикла, а то повторение одного и того же кода... Неправильо это. 
                        // 04.04 - убрал я вашу индусятину, но пришлось щипотку своей добавить
                        while (!resultCheckUser && i < MAX_COUNTER_ATTEMPT)
                        {
                            Sleep(latency);

                            if (firstTimeFlag || Pipes[PipeNumber].ReadMessage(Message)) // если данные есть в канале
                            {
                                firstTimeFlag = false; // немного по индусски, но и ваш код был не очень
                                if (GetLastError() == CLIENT_DISCONNECT)
                                    break;

                                PipeInfo[PipeNumber].ReadVal(Message); // записываем их в вектор
                                tempData = PipeInfo[PipeNumber].getData(); // получаем элемент вектора
                                std::cout << "Testing Message. Write Response" << std::endl;
                                resultCheckUser = Pipes[PipeNumber].checkUser(tempData); // сверяем логин и пароль с базой
                                Pipes[PipeNumber].WriteResponse(resultCheckUser); // отправляем результат проверки

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


                std::cout << "Ожидание подключения клиентов..." << std::endl;
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