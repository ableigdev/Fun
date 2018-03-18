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
		char* str = new char[strlen(Val) + 1];
		strcpy(str, Val);
		data.push_back(str);
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