#define _CRT_SECURE_NO_WARNINGS
#pragma once

#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>

#include "User.h"

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
		
	std::vector<User<T>> data;

public:

	//---------------------------------------------------------------
	/*
	��������� ������������ �����, ������� � ����������� �� �������� ���� ControlInd
	���������� ��������� �������� ��� � ������ ControlVals, ��� � ������.
	*/

	void ReadVal(std::basic_string<T> str)
	{
		data.push_back(parseString(str));
	}

	//---------------------------------------------------------------
	/*
	��������� ������������ �����, ������� ������� ������ ControlVals � ������.
	*/

	void ClearData()
	{
		data.clear();
	}

	User<T> parseString(std::basic_string<T> str)
	{
		User<T> user;
		size_t index = str.find_first_of("/");
        size_t eOF = str.find('\0');
		user.login = str.substr(0, index);
		user.password = str.substr(index + 1, eOF - (index + 1)); //looks like a shit :D

		return user;
	}

	std::vector<User<T>> getData() const
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