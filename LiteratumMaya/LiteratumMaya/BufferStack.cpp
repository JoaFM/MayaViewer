#include "BufferStack.h"



BufferStack::BufferStack()
{
	
}


BufferStack::~BufferStack()
{


}

void BufferStack::Add(char* Dataparam, int size)
{
	for (int i = 0; i < size; i++)
	{
		DataStack.push_back(Dataparam[i]);
	}

}

int BufferStack::Num() const
{
	return (int)DataStack.size();
}

bool BufferStack::GetData(int size, std::vector<char>& returnData)
{
	if (size > Num()) return false;


	returnData.resize(size);

	for (int i = 0; i < size; i++)
	{
		returnData[i] = DataStack.front();
		DataStack.pop_front();
	}
	return true;

}
