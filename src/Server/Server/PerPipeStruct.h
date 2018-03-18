#pragma once

#include "list.h"
#include <iostream>

//-----------------------------------------------------------------------

#define MAX_LOG_PASS_LENGTH 50

template <typename T> 
class PerPipeStruct
{
private:

	// Описание недоступных пользователю полей класса
	/*
	ControlVals - массив из 3-ех элементов, содержащий начальное значение, конечное значение и
	шаг изменения значения.
	ControlInd - количество заполненных элементов массива ControlVals.
	List - экземпляр класса для работы с линейным однонаправленным списком, в который
	записываются полученные из потока значения, кратные заданному делителю в заданном
	диапазоне
	*/
	
	char* loginServer;
	char* passwordServer;
	unsigned ControlInd;
	TList<T> List;

public:

	PerPipeStruct()
		: loginServer(new char[MAX_LOG_PASS_LENGTH]),
		passwordServer(new char[MAX_LOG_PASS_LENGTH])
	{
		//ClearData();
	}

	//---------------------------------------------------------------
	/*
	Доступный пользователю метод, который в зависимости от значения поля ControlInd
	записывает очередное значение или в массив ControlVals, или в список.
	*/

	void ReadVal(T Val)
	{
		loginServer = Val;

	}

	//---------------------------------------------------------------
	/*
	Доступный пользователю метод, который очищает массив ControlVals и список.
	*/

	void ClearData()
	{
		delete[] loginServer;
		delete[] passwordServer;
	}

	//---------------------------------------------------------------
	template <typename T>
	friend std::ostream& operator << (std::ostream &os, PerPipeStruct<T> &Val);
};

//-----------------------------------------------------------------------

template <typename T> 
std::ostream& operator << (std::ostream &os, PerPipeStruct<T> &Val)
{
	os << Val.loginServer << " " << Val.passwordServer << std::endl;

	return os;
}