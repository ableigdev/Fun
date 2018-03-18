#define _CRT_SECURE_NO_WARNINGS
#pragma once

#include <iostream>
#include <vector>
#include <cstdlib>

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
	
	std::vector<T> data;

public:

	PerPipeStruct()
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
		char* str = new char[strlen(Val) + 1];
		strcpy(str, Val);
		data.push_back(str);
	}

	//---------------------------------------------------------------
	/*
	Доступный пользователю метод, который очищает массив ControlVals и список.
	*/

	void ClearData()
	{
		data.clear();
	}

	//---------------------------------------------------------------
	template <typename T>
	friend std::ostream& operator << (std::ostream &os, PerPipeStruct<T> &Val);
};

//-----------------------------------------------------------------------

template <typename T> 
std::ostream& operator << (std::ostream &os, PerPipeStruct<T> &Val)
{
	for (T x : Val.data)
	{
		os << x << " ";
	}

	return os;
}