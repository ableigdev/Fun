#pragma once

#include "list.h"
#include <iostream>

//-----------------------------------------------------------------------

#define	NELEM	3

template <typename T> 
class PerPipeStruct
{
private:

	// �������� ����������� ������������ ����� ������
	/*
	ControlVals - ������ �� 3-�� ���������, ���������� ��������� ��������, �������� �������� �
	��� ��������� ��������.
	ControlInd - ���������� ����������� ��������� ������� ControlVals.
	List - ��������� ������ ��� ������ � �������� ���������������� �������, � �������
	������������ ���������� �� ������ ��������, ������� ��������� �������� � ��������
	���������
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
	��������� ������������ �����, ������� � ����������� �� �������� ���� ControlInd
	���������� ��������� �������� ��� � ������ ControlVals, ��� � ������.
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
	��������� ������������ �����, ������� ������� ������ ControlVals � ������.
	*/

	void ClearData()
	{
		for (ControlInd = 0; ControlInd < NELEM; ControlInd++)
			ControlVals[ControlInd] = 0;
		ControlInd = 0;
		List.DelAllElem();
	}

	//---------------------------------------------------------------

	friend std::ostream& operator << (std::ostream &os, PerPipeStruct &Val);
};

//-----------------------------------------------------------------------

template <typename T> std::ostream& operator << (std::ostream &os, PerPipeStruct<T> &Val)
{
	if (Val.ControlInd > 0)
	{
		os << endl << "��������� ��������, �������� ��������, ���:" << endl;
		os << Val.ControlVals[0] << " " << Val.ControlVals[1] << " " << Val.ControlVals[2] << endl;
		os << "�������� ��������: " << endl;
		os << Val.List;
	}

	return os;
}