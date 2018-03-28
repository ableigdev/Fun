#pragma once

#include <windows.h>
#include <cstdlib>
#include <string>

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
			size_t sizePipe = strlen(PipeName) + 1;

			std::wstring wPipeName(sizePipe, L'#');
			mbstowcs(&wPipeName[0], PipeName, sizePipe);

			hPipe = CreateFile(wPipeName.c_str(),
				GENERIC_READ | GENERIC_WRITE,
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

					if (!WaitNamedPipe(wPipeName.c_str(), WaitInfinite ? NMPWAIT_WAIT_FOREVER : NMPWAIT_USE_DEFAULT_WAIT))
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

	bool WriteMessage(std::basic_string<T> &Message)
	{
		if (IsPipeConnected())
		{
			DWORD NBWr;
			return WriteFile(hPipe, &Message.at(0), 100, &NBWr, NULL) == TRUE;
		}
		return false;
	}

	//------------------------------------------------------------------

	bool ReadResponse()
	{
		if (IsPipeConnected())
		{
			DWORD NBytesRead;
			bool Message;
			ReadFile(hPipe, &Message, 1, &NBytesRead, NULL);
			
			return Message;
		}
		return false;
	}
	/*
	Доступный пользователю метод, с помощью которого осуществляется проверка удачности
	подключения к экземпляру именованного канала
	*/

	bool IsPipeConnected()
	{
		return hPipe != INVALID_HANDLE_VALUE;
	}

	//------------------------------------------------------------------
	/*
	Доступный пользователю метод, с помощью которого осуществляется подключение к серверу и передача
	*/


	void ConnectToServer(char *PipeName, std::basic_string<T>& login, std::basic_string<T>& password)
	{
		if (ConnectPipe(PipeName))
		{
			InitMessageMode();

			std::basic_string<T> str(login + "/" + password);

			if (!(WriteMessage(str)))
			{
				std::cout << "\nОшибка записи в именованный канал!\n";
			}

			do//будем вводить логин и пароль пока не получим успешную авторизацию
			{
				std::cout << "\nВведите логин: ";
				std::cin >> login;

				std::cout << "Введите пароль: ";
				std::cin >> password;
				//InitMessageMode();

				if (!(WriteMessage(login) && WriteMessage(password)))
				{
					std::cout << "\nОшибка записи в именованный канал!\n";
					break;
				}

				if (ReadResponse())
				{
					std::cout << "\nАвторизация прошла успешно!\n";
					break;
				}
				else
				{
					std::cout << "\nНеверный пароль или логин!\n";
					std::cout << "Повторить ввод? (N - прекратить вход)";
					char answ = 'y';
					std::cin >> answ; 
					if (answ == 'N' || answ == 'n')
					{
						break;
					}
				}
			} while (IsPipeConnected());

			std::cout << "\nНажмите любую клавишу для завершения программы (выполнится отключение от именованного канала " << PipeName << ")\n";
		}
		else
			std::cout << "\nОшибка соединения с сервером (код ошибки: " << GetLastError() << ")!\n";

	}

	//------------------------------------------------------------------
	/*
	Доступный пользователю метод, с помощью которого осуществляется попытка авторизации
	*/

	bool authorization(bool serverAnswer = 0)
	{
		
	}

};