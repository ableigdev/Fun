#pragma once

#include "list.h"
#include <iostream>

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
	��������� ������������ �����, ������� � ����������� �� �������� ���� ControlInd
	���������� ��������� �������� ��� � ������ ControlVals, ��� � ������.
	*/

	void ReadVal(T Val)
	{
		loginServer = Val;

	}

	//---------------------------------------------------------------
	/*
	��������� ������������ �����, ������� ������� ������ ControlVals � ������.
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