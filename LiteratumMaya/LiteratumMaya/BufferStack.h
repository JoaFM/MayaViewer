#pragma once


#include <stack>
#include <iostream>
#include <vector>
#include <list>

class BufferStack
{
public:
	BufferStack();
	~BufferStack();

	void Add(char* Dataparam, int size);
	int Num() const;
	bool BufferStack::GetData(int size, std::vector<char>& returnData);

private:
	std::list<char> DataStack;

};

