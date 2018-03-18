#pragma once

#include <iostream>

template <typename INF, typename LISTTYPE> class Iterator;

// ��������� ����� ��� ������ � ���������������� �������
template <typename INF> 
class TList
{
	friend	class Iterator <INF, TList<INF > >;

	// ��� ������, ������� ��������� ������� ������
	// ������������� ����� (�������� ��������) + ��������� �� ��������� ������� (����� �� ����� NULL, �� ��� ��������� �������)

	template <typename ElemType> struct TElem
	{
		ElemType Inf;
		TElem *Next;

	};

	typedef TElem<INF> ListElem;

protected:

	// ��������� ��������� �� ������ � ��������� ��������, ������� ���������� ������������
	TElem<INF> *Head, *Tail;

public:

	typedef Iterator <INF, TList<INF > > IteratorTList;

	TList()
	{
		Head = Tail = NULL;
	}

	~TList()
	{
		DelAllElem();
	}

	//-----------------------------------------------------------------------

	// ������� ������ �������� ����� ����������. ������� �������� ����� �������������� ������

	void AddAfterTail(INF Inf)
	{
		ListElem *Cur = new TElem<INF>;

		Cur->Inf = Inf;
		Cur->Next = NULL;
		if (Head != NULL)
			Tail->Next = Cur;
		else
			Head = Cur;
		Tail = Cur;
	}

	// ������� ������ �������� ����� ����������. ������� �������� ����� �������������� ������
	// ��� ���������� ������ � ������������� ����� ����������� interlocked-�������

	void AddAfterTailInterlocked(INF Inf)
	{
		ListElem *Cur = new TElem<INF>;

		Cur->Inf = Inf;
		Cur->Next = NULL;
		if (Head != NULL)
			InterlockedExchange((LONG*)(&(Tail->Next)), (LONG)Cur);
		else
			InterlockedExchange((LONG*)(&Head), (LONG)Cur);

		InterlockedExchange((LONG*)(&Tail), (LONG)Cur);

	}



	//-----------------------------------------------------------------------

	// ����� ��������. ������� �������� ����� �������������� ������

	bool SearchElement(INF Inf, IteratorTList &It)
	{
		for (It = *this; !It && !(It == Inf); ++It);
		return It.GetCurInf(Inf);

	}

	//-----------------------------------------------------------------------

	// �������� ������������� ��������� � ������

	bool IsNotEmpty()
	{
		return (Head != NULL);
	}

	//-----------------------------------------------------------------------

	// �������� ���� ��������� �� ������

	void DelAllElem()
	{
		ListElem *Cur = Head;

		while (Head != NULL)
		{
			Cur = Head;
			Head = Head->Next;
			delete Cur;
		}

		Tail = NULL;
	}

	template <typename INF>
	friend std::ostream& operator << (std::ostream &os, TList<INF> &List);

};


//-----------------------------------------------------------------------


template <typename INF> 
std::ostream& operator << (std::ostream &os, TList<INF> &List)
{
	INF *Val = NULL;

	for (TList<INF>::IteratorTList It = List; !It; ++It)
	{
		It.GetCurInf(Val);
		os << *Val << std::endl;
	}

	return os;
}

//-----------------------------------------------------------------------

template <typename INF, typename LISTTYPE> 
class Iterator
{
	typename LISTTYPE::ListElem **Head, **Tail, *CurPos;
	
public:

	Iterator()
	{
		Head = Tail = CurPos = NULL;
	}

	Iterator(Iterator &It)
	{
		*this = It;
	}

	Iterator(LISTTYPE &List)
	{
		*this = List;
	}

	//---------------------------------------------------

	Iterator& operator = (Iterator &It)
	{
		Head = It.Head;
		Tail = It.Tail;
		CurPos = It.CurPos;
		return *this;
	}

	Iterator& operator = (LISTTYPE &List)
	{
		Head = &(List.Head);
		Tail = &(List.Tail);
		CurPos = List.Head;
		return *this;
	}

	Iterator& operator = (INF &Val)
	{
		if (CurPos)
			CurPos->Inf = Val;

		return *this;
	}

	//---------------------------------------------------

	Iterator& operator ++()
	{
		if (CurPos)
			CurPos = CurPos->Next;

		return *this;
	}

	//---------------------------------------------------

	bool operator !()
	{
		return CurPos != NULL;
	}

	bool operator == (INF Val)
	{
		return CurPos ? CurPos->Inf == Val : false;
	}

	//---------------------------------------------------

	void SetStart()
	{
		CurPos = *Head;
	}

	bool IsNotEmpty()
	{
		return (*Head != NULL);
	}

	//---------------------------------------------------

	bool GetCurInf(INF &Inf)
	{
		if (CurPos)
		{
			Inf = CurPos->Inf;
			return true;
		}
		return false;
	}

	bool GetCurInf(INF *&Inf)
	{
		if (CurPos)
		{
			Inf = &(CurPos->Inf);
			return true;
		}
		return false;
	}

};
