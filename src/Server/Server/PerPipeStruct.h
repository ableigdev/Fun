#pragma once

#include "list.h"
#include <iostream>

//-----------------------------------------------------------------------

#define	NELEM	3

template <template T> class PerPipeStruct
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

	T ControlVals[NELEM];
	unsigned ControlInd;
	TList<T> List;

public:

	PerPipeStruct()
	{
		ClearData();
	}

	//---------------------------------------------------------------
	/*
	Доступный пользователю метод, который в зависимости от значения поля ControlInd
	записывает очередное значение или в массив ControlVals, или в список.
	*/

	void ReadVal(T Val)
	{
		if (ControlInd < NELEM)
			ControlVals[ControlInd++] = Val;
		else
			List.AddAfterTail(Val);

	}

	//---------------------------------------------------------------
	/*
	Доступный пользователю метод, который очищает массив ControlVals и список.
	*/

	void ClearData()
	{
		for (ControlInd = 0; ControlInd < NELEM; ControlInd++)
			ControlVals[ControlInd] = 0;
		ControlInd = 0;
		List.DelAllElem();
	}

	//---------------------------------------------------------------

	friend ostream& operator << (ostream &os, PerPipeStruct &Val);
};

//-----------------------------------------------------------------------

template <template T> ostream& operator << (ostream &os, PerPipeStruct<T> &Val)
{
	if (Val.ControlInd > 0)
	{
		os << endl << "Начальное значение, конечное значение, шаг:" << endl;
		os << Val.ControlVals[0] << " " << Val.ControlVals[1] << " " << Val.ControlVals[2] << endl;
		os << "Числовые значения: " << endl;
		os << Val.List;
	}

	return os;
}