#define _CRT_SECURE_NO_WARNINGS
#pragma once

#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>

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
	
	std::vector<std::basic_string<T>> data;

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

	void ReadVal(T* str)
	{
		data.push_back(std::basic_string<T>(str));
	}

	//---------------------------------------------------------------
	/*
	Доступный пользователю метод, который очищает массив ControlVals и список.
	*/

	void ClearData()
	{
		data.clear();
	}

	std::vector<std::basic_string<T>> getData() const
	{
		return data;
	}

	//---------------------------------------------------------------
	template <typename T>
	friend std::ostream& operator << (std::ostream&, PerPipeStruct<T>&);

	template <typename T>
	friend std::wostream& operator << (std::wostream&, PerPipeStruct<T>&);
};

//-----------------------------------------------------------------------

template <typename T> 
std::ostream& operator << (std::ostream& ostream, PerPipeStruct<T>& right)
{
	for (size_t i = 0; i < right.data.size(); ++i)
	{
		ostream << right.data[i] << " ";
	}

	return ostream;
}

template <typename T>
std::wostream& operator << (std::wostream& wostream, PerPipeStruct<T>& right)
{
	for (size_t i = 0; i < right.data.size(); ++i)
	{
		wostream << right.data[i] << " ";
	}

	return wostream;
}