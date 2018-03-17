#pragma once

#include <windows.h>

template <typename T> 
class CPipeClient
{

	/*
	Описание недоступного пользователю поля, в котором будет храниться дескриптор именованного канала
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
	Доступный пользователю метод, с помощью которого осуществляется попытка открытия
	именованного канала с указанным именем (аргумент PipeName) с помощью функции CreateFile
	для записи в него данных. Далее в случае необходимости выполняется ожидание готовности
	сервера в течение указанного промежутка времени (если аргумент WaitInfinite равен false,
	то происходит бесконечное ожидание, а в противном случае ожидание выполняется в течение
	времени, указанного при создании экземпляра потока сервером).
	*/

	bool ConnectPipe(char *PipeName, bool WaitInfinite = false)
	{
		/*
		Бесконечный цикл попыток подключения
		*/
		do
		{
			/*
			Попытка подключения к каналу
			*/

			hPipe = CreateFile((wchar_t*)PipeName,
				GENERIC_WRITE,
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
					Если подключение не произошло по причине занятости сервера, то выполнение ожидания его
					освобождения (в случае неудачности ожидания   выход) и переход на повторное подключение
					*/

					if (!WaitNamedPipe((wchar_t*)PipeName, WaitInfinite ? NMPWAIT_WAIT_FOREVER : NMPWAIT_USE_DEFAULT_WAIT))
						return false;
				}
				else
					// Другая ошибка при подключении, следовательно, оно не удалось
					return false;
			}
			else
				// Удачное подключение
				break;
		} while (1);

		return true;
	}

	//------------------------------------------------------------------
	/*
	Доступный пользователю метод, с помощью которого осуществляется попытка перевода канала,
	если подключение к нему выполнено успешно, в режим сообщений
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
	Доступный пользователю метод, с помощью которого осуществляется попытка записи данных в
	канал, если подключение к нему выполнено успешно.
	*/

	bool WriteMessage(T &Message)
	{
		if (IsPipeConnected())
		{
			DWORD NBWr;
			return WriteFile(hPipe, (LPVOID)(&Message), sizeof(Message), &NBWr, NULL) == TRUE;
		}
		return false;
	}

	//------------------------------------------------------------------
	/*
	Доступный пользователю метод, с помощью которого осуществляется проверка удачности
	подключения к экземпляру именованного канала
	*/

	bool IsPipeConnected()
	{
		return hPipe != INVALID_HANDLE_VALUE;
	}

};