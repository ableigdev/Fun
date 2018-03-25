#include <iostream>
#include <fstream>
#include <Windows.h>

#include "PipeServer.h"
#include "PerPipeStruct.h"

#define MAX_PIPE_INST	2
#define PIPE_NAME		L"\\\\.\\pipe\\pipe_example"

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
	char *FName = new char[MAX_PATH], answer;
	std::ofstream file;
	DWORD PipeNumber, NBytesRead;
	char* Message = new char[100];
	int PipesConnect = 0;

	SetConsoleOutputCP(1251);
	std::cout << "Введите имя выходного файла: ";
	std::cin >> FName;

	file.open(FName);
	if (!file)
		std::cout << "Ошибка создания файла с именем " << FName << "!" << std::endl;
	else
	{
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

			/*
			Ожидания перехода в свободное состояние события, связанного с каким-то из каналов,
			что свидетельствует о завершении асинхронной операции, и получения номера этого канала.
			При этом функция WaitForMultipleObjects состояние события с ручным сбросом
			АВТОМАТИЧЕСКИ НЕ ИЗМЕНЯЕТ.
			*/

			PipeNumber = WaitForMultipleObjects(MAX_PIPE_INST, hEvents, FALSE, INFINITE) - WAIT_OBJECT_0;

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

						if (Pipes[PipeNumber].ReadMessage(Message))
						{
							/*
							Если завершена асинхронная операция чтения, то проверка состояния операции
							*/

							switch (Pipes[PipeNumber].GetOperState())
							{
								/*
								Если прочитаны не все данные сообщения, то запуск повторного чтения
								*/

							case PIPE_READ_PART:		Pipes[PipeNumber].ReadMessage(Message);
								std::cout << "Testing Message. Reading data part" << std::endl;
								break;

								/*
								Если чтение сообщения завершено успешно, то обработка полученного значения
								*/

							case PIPE_READ_SUCCESS:		PipeInfo[PipeNumber].ReadVal(Message);
								std::cout << "Testing Message. Reading data" << std::endl;
								break;

								/*
								Произошла ошибка чтения
								*/
							case PIPE_OPERATION_ERROR:	std::cout << "Ошибка при чтении данных из канала (код ошибки: " << GetLastError() << ")!" << std::endl;
								break;

							}
							break;
						}
						//break;

						/*
						Отключение клиента. В этом случае происходит вывод данных, прочитанных из канала в файл и
						их очистка в структуре данных, уменьшения количество подключенных клиентов на 1,
						отключение клиента со стороны сервера и запуск ожидания подключения нового клиента
						*/

					case PIPE_LOST_CONNECT:		
						file << PipeInfo[PipeNumber];
						std::cout << "Testing Message. File Write" << std::endl;
						PipeInfo[PipeNumber].ClearData();
						if (PipesConnect > 0)
							PipesConnect--;
						Pipes[PipeNumber].DisconnectClient();
						Pipes[PipeNumber].WaitClient();
						if (Pipes[PipeNumber].CanClose() == false)
						{
							std::cout << "В канал не было передано никаких данных со стороны клиента. Повторить попытку чтения данныхY или y - да / любая другая клавиша - нет)?" << std::endl;
							std::cin >> answer;
							if (answer == 'Y' || answer == 'y')
								continue;
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
					break;
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