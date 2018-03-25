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

	// �������� ����������� ������������ ����� ������
	/*
	ControlVals - ������ �� 3-�� ���������, ���������� ��������� ��������, �������� �������� �
	��� ��������� ��������.
	ControlInd - ���������� ����������� ��������� ������� ControlVals.
	List - ��������� ������ ��� ������ � �������� ���������������� �������, � �������
	������������ ���������� �� ������ ��������, ������� ��������� �������� � ��������
	���������
	*/
	
	std::vector<std::basic_string<T>> data;

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

	void ReadVal(T* str)
	{
		data.push_back(std::basic_string<T>(str));
	}

	//---------------------------------------------------------------
	/*
	��������� ������������ �����, ������� ������� ������ ControlVals � ������.
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