#pragma once

#include "list.h"
#include <iostream>
#include <vector>

//-----------------------------------------------------------------------

#define MAX_LOG_PASS_LENGTH 50

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
	
	std::vector<T> data;
	unsigned ControlInd;
	TList<T> List;

public:

	PerPipeStruct()
	{
		//ClearData();
	}

	//---------------------------------------------------------------
	/*
	��������� ������������ �����, ������� � ����������� �� �������� ���� ControlInd
	���������� ��������� �������� ��� � ������ ControlVals, ��� � ������.
	*/

	void ReadVal(T Val)
	{
		T value = Val;
		data.push_back(value);
	}

	//---------------------------------------------------------------
	/*
	��������� ������������ �����, ������� ������� ������ ControlVals � ������.
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