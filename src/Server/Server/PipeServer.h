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
    // Описание недоступных пользователю полей класса

    HANDLE readEvent;

    // Дескриптор именованного канала

    HANDLE hPipe;

    // Экземпляр структуры OVERLAPPED для осуществления работы в асинхронном режиме
    OVERLAPPED Overl;

    /*
    Поле, содержащее состояние именованного канала. Возможные значения:
    PIPE_ERROR - ошибка при работе с именованным каналом. Использование канала невозможно
    PIPE_NOT_CONNECTED - клиент к каналу не подключен. Использование канала невозможно
    PIPE_CONNECTED - клиент подключился к каналу. Канал может быть использован
    PIPE_LOST_CONNECT - клиент отключился от канала. Использование канала невозможно
    */
    DWORD PipeState;

    /*
    Флаг, определяющий незаконченность асинхронной операции ввода-вывода:
    true - асинхронная операция ввода-вывода завершена
    false - асинхронная операция ввода-вывода в процессе выполнения
    */
    bool fPendingIOComplete;
    bool CanCloseFlag;
    /*
    Поле, содержащее состояние текущей операции в именованном канале:
    PIPE_OPERATION_ERROR - ошибка выполнения операции
    PIPE_JUST_CONNECTED - никакой операции в канале не выполняется, т. к. клиент только что подключился
    PIPE_NO_OPERATION - никакой операции в канале не выполняется либо асинхронная операция еще не закончена
    PIPE_READ_SUCCESS - операция асинхронного чтения успешно завершена
    PIPE_READ_PART - операция асинхронного чтения успешно завершена, однако прочитана лишь часть сообщения
    */
    DWORD PipeCurOperState;

    /*
    Указатель на структуру данных, описывающую дескриптор безопасности. Для того чтобы к
    серверу именованного канала можно было подключиться по сети необходимо создать
    соответствующий дескриптор безопасности, позволяющий всем пользователям (от имени которых
    запускаются клиенты) осуществлять запись в именованный канал сервера. В противном случае
    потребовался бы запуск сервера и клиента от имени одного и того же пользователя.
    */

    PSECURITY_DESCRIPTOR pSD;

    std::vector<User<T>> db_users;
    int maxLenLog, maxLenPass;

    //------------------------------------------------------------------

    /*
    Недоступный пользователю метод класса, который в зависимости от кода, возвращаемого функцией GetLastError,
    изменяет поле состояния канала
    */

    void CheckError()
    {
        DWORD Error = GetLastError();

        switch (Error)
        {
            // Асинхронная операция в процессе выполнения
        case ERROR_IO_PENDING:			PipeCurOperState = PIPE_NO_OPERATION;
            fPendingIOComplete = false;

            break;

            // Клиент отключился от именованного канала
        case ERROR_BROKEN_PIPE:			PipeState = PIPE_LOST_CONNECT;
            fPendingIOComplete = true;
            break;

            // Произошла ошибка при работе с каналом 
        default:						PipeCurOperState = PIPE_OPERATION_ERROR;
            fPendingIOComplete = true;
            break;

        }
    }

    //------------------------------------------------------------------

    // Начальная инициализация полей экземпляра структуры OVERLAPPED
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
            // Отмена всех асинхронных операций
            CancelIo(hPipe);
            CloseHandle(hPipe);
        }

        /*
        Освобождение в случае необходимости памяти, выделенной с помощью функции HeapAlloc под дескриптор безопасности, которое осуществляется с помощью функции HeapFree, прототип которой описан в файле заголовков winbase.h следующим образом:

        BOOL HeapFree(
        HANDLE hHeap,
        DWORD dwFlags,
        LPVOID lpMem
        );

        где hHeap - дескриптор кучи, который в примере получается с помощью функции GetProcessHeap,
        которая возвращает дескриптор кучи, вызвавшего ее процесса (прототип функции описан в файле
        заголовков winbase.h). Аргумент dwFlags может содержать значение HEAP_NO_SERIALIZE
        (соответствует числовому значению 1). Эта константа означает, что не будет производиться
        ограничения доступа при попытке нескольких потоков освободить память. Аргумент lpMem
        задает адрес, начиная с которого будет освобождена память. В случае удачного выполнения
        функция возвращает не 0, а иначе - 0.
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
    Доступный пользователю метод класса, который создает экземпляр именованного канала с
    указанным именем (PipeName), записывая полученный дескриптор в поле hPipe. Аргумент
    BufSize задает размер буферов ввода-вывода, аргумент DefWaitTime задает время ожидания
    соединения клиента с каналом по умолчанию, аргумент ByteMode при своем значении true задает
    байтовый режим, а в противном случае - режим сообщений для создаваемого именованного
    канала. Кроме этого функция связывает экземпляр структуры OVERLAPPED с дескриптором
    события (аргумент hEvent), которое должно быть создано. После создания именованного
    канала происходит запуск асинхронного ожидания подключения клиента к каналу
    (метод WaitClient, который изменяет поле состояния канала)
    */

    DWORD CreatePipeAndWaitClient(wchar_t *PipeName, HANDLE hEvent, bool ByteMode = true, DWORD BufSize = DEF_BUF_SIZE, DWORD DefWaitTime = DEF_WAIT_TIME)
    {
        if (hEvent)
        {
            ClearOVERL();
            Overl.hEvent = hEvent;

            /*
            Выделение памяти под дескриптор безопасности с помощью функции HeapAlloc, прототип
            которой описан в файле заголовков winbase.h следующим образом:

            LPVOID HeapAlloc(
            HANDLE hHeap,
            DWORD dwFlags,
            SIZE_T dwBytes
            );

            где hHeap - дескриптор кучи, который в примере получается с помощью функции GetProcessHeap.
            Аргумент dwFlags позволяет с помощью специальных констант управлять процессом выделения
            памяти. В качестве этого аргумента, кроме 0, можно передавать следующие константные значения:
            HEAP_GENERATE_EXCEPTIONS (числовое значение 4) - означает, что в случае неудачного выполнения
            будет сгенерировано соответствующее исключение (если этот флаг не задать, то в описанной
            ситуации функция вернет NULL);
            HEAP_NO_SERIALIZE (числовое значение 1) - описание см. выше;
            HEAP_ZERO_MEMORY (числовое значение 8) - выделенная память будет заполнена нулями.
            Аргумент dwBytes задает размер выделяемой памяти в байтах. В случае удачного выполнения
            функция вернет указатель на выделенный фрагмент памяти.
            */

            if (!pSD)
                pSD = HeapAlloc(GetProcessHeap(), HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, SECURITY_DESCRIPTOR_MIN_LENGTH);

            if (pSD && InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION) && SetSecurityDescriptorDacl(pSD, TRUE, NULL, FALSE))
            {

                /*
                Если инициализация и заполнение данными DACL дескриптора безопасности прошли успешно, то
                производится описание и соответствующая инициализация экземпляра класса SECURITY_ATTRIBUTES,
                который будет задействован при вызове функции CreateNamedPipe
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
    Доступный пользователю метод класса, который в случае, когда имеется валидный дескриптор
    канала (проверяется методом IsOpen) осуществляет асинхронное ожидание подключения к каналу
    и изменяет соответствующим образом поле состояния канала
    */


    DWORD WaitClient()
    {
        if (IsOpen())
        {
            // Операций в канале нет
            PipeCurOperState = PIPE_NO_OPERATION;
            ClearOVERL();

            // Асинхронное ожидание подключения клиента к каналу
            if (ConnectNamedPipe(hPipe, &Overl) == FALSE)
            {
                switch (GetLastError())
                {
                    // Выполняется ожидание подключения клиента к каналу в асинхронном режиме
                case ERROR_IO_PENDING:		fPendingIOComplete = false;
                    return PipeState;

                    // Клиент подключился к каналу																
                case ERROR_PIPE_CONNECTED:
                    // Перевод события в свободное состояние
                    SetEvent(Overl.hEvent);
                    PipeState = PIPE_CONNECTED;
                    CanCloseFlag = false;
                    fPendingIOComplete = true;
                    return PipeState;

                }
            }
            /*
            Ожидание подключения клиента к каналу в асинхронном режиме завершено неудачно
            */
            CanCloseFlag = false;
            fPendingIOComplete = true;
        }
        PipeState = PIPE_ERROR;

        return PipeState;
    }

    //------------------------------------------------------------------

    /*
    Доступный пользователю метод класса, который в случае, когда имеется валидный дескриптор
    канала (проверяется методом IsOpen) осуществляет отключение клиента от канала
    (выполняется на стороне сервера в случае, когда клиент завершил использование канала).
    При этом в зависимости от успешности выполнения действия изменяется поле состояния канала
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
    Доступный пользователю метод класса, который в случае, когда имеется валидный дескриптор
    канала (проверяется методом IsOpen) осуществляет запуск асинхронной операции чтения данных
    (данные рано или поздно будут записаны в аргумент Message)
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
                    //произошла какая-то ошибка при чтении из пайпа
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
                    //Если чтение произошло успешно
                    std::cout << "Testing Message. Receiving data from client. \n";
                    Message[NBytesRead] = '\0'; //making end of string for parser
                    CanCloseFlag = true;
                    fPendingIOComplete = true;
                    PipeCurOperState = PIPE_JUST_CONNECTED;

                    return true;
                }
                else
                {
                    //Операция была выполнена асинхронно, но что-то пошло не так
                    CancelIo(hPipe);
                    std::cout << "error reading file \n";

                }
            }
            else
            {
                //Если чтение произошло успешно
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

                std::cout << "Ошибка записи в канал " << GetLastError();
            }
        }
    }

    //------------------------------------------------------------------

    /*
    Доступный пользователю метод класса, который в случае, когда имеется валидный дескриптор
    канала (проверяется методом IsOpen) осуществляет проверку завершения асинхронной операции
    и получение количества принятой информации
    */

    bool GetPendingResult(DWORD &NBytesRead)
    {

        /*
        Если асинхронная операция завершена успешно, то изменение полей состояния канала и
        состояния текущей операции в канале в случае необходимости
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
            В противном случае проверка состояния канала и изменение поля его состояния
            */
            CheckError();
        }

        return false;
    }

    //------------------------------------------------------------------

    /*
    Доступный пользователю метод класса, который проверяет удачность создания экземпляра
    именованного канала
    */

    bool IsOpen()
    {
        return hPipe != INVALID_HANDLE_VALUE;
    }

    //------------------------------------------------------------------

    /*
    Доступный пользователю метод класса, который возвращает значение поля состояния
    экземпляра канала
    */

    DWORD GetState()
    {
        return PipeState;
    }

    //------------------------------------------------------------------

    /*
    Доступный пользователю метод класса, который возвращает значение поля состояния
    текущей операции для этого экземпляра канала
    */

    DWORD GetOperState()
    {
        return PipeCurOperState;
    }

    //------------------------------------------------------------------
    /*
    Доступный пользователю метод класса, который возвращает значение признака
    завершения асинхронной операции для этого экземпляра канала
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