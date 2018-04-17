#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h> 

//#include "Game.h"
#include "Renderer.h"

#include "Engine.h"


//struct MappedBuffer
//{
//	//VkBuffer handle;
//	//VkDeviceMemory memory;
//
//	//VkDeviceSize size;
//	uint64_t size;
//	//VkBufferUsageFlags usage;
//	uint64_t fragments;
//
//	struct Node
//	{
//		Node* prev;
//		Node* next;
//		//VkDeviceSize offset;
//		uint64_t offset;
//		//VkDeviceSize size;
//		uint64_t size;
//		bool inUse;
//	};
//	Node* subBufferMapHead;
//	Node* subBufferMapTail;
//
//	void Init(uint64_t _size/*, VkBufferUsageFlags _usage, VkDevice _vkDevice, PhysicalDevice _physicalDevice, VkMemoryPropertyFlags _memoryProperties*/)
//	{
//		size = _size;
//
//		subBufferMapHead = new MappedBuffer::Node;
//		fragments = 1;
//		subBufferMapTail = subBufferMapHead;
//
//		subBufferMapHead->inUse = false;
//		subBufferMapHead->offset = 0;
//		subBufferMapHead->size = size;
//		subBufferMapHead->prev = nullptr;
//		subBufferMapHead->next = nullptr;
//
//		// Create GPU buffer
//	}
//	Node* AllocateSubBuffer(uint64_t _size/*, void* _data, VkCommandBuffer _commandBuffer, VkDevice _device, PhysicalDevice _physicalDevice*/)
//	{
//		// Find subBuffer to allocate from
//		MappedBuffer::Node* currentBuffer = subBufferMapHead;
//		for (;;)
//		{
//			if (currentBuffer == nullptr)
//				return nullptr; // No space available
//			if (currentBuffer->inUse == true || currentBuffer->size < _size)
//			{
//				currentBuffer = currentBuffer->next;
//				continue; // Check if next is available
//			}
//
//			break; // Found space
//		}
//
//		// Update buffer map
//		MappedBuffer::Node* newBuffer;
//
//		// if fragment has the same size
//		if (currentBuffer->size == _size)
//		{
//			newBuffer = currentBuffer;
//			newBuffer->inUse = true;
//		}
//		// if fragment must be split
//		else
//		{
//			newBuffer = new MappedBuffer::Node;
//			++fragments;
//			newBuffer->next = currentBuffer;
//			newBuffer->prev = currentBuffer->prev;
//			newBuffer->offset = currentBuffer->offset;
//			newBuffer->size = _size;
//			newBuffer->inUse = true;
//
//			if (newBuffer->prev != nullptr)
//				newBuffer->prev->next = newBuffer;
//			else
//				subBufferMapHead = newBuffer;
//			if (newBuffer->next != nullptr)
//				newBuffer->next->prev = newBuffer;
//
//			currentBuffer->size -= newBuffer->size;
//			currentBuffer->offset += newBuffer->size;
//		}
//
//		// Fill GPU bufer
//
//
//		return newBuffer;
//	}
//	void FreeSubBuffer(Node* _subBuffer/*, VkCommandBuffer _commandBuffer, VkDevice _device, PhysicalDevice _physicalDevice*/)
//	{
//		_subBuffer->inUse = false;
//
//		if (_subBuffer->next != nullptr)
//		{
//			if (_subBuffer->next->inUse == false)
//			{// Join next
//				MappedBuffer::Node* tempNode = _subBuffer->next;
//				_subBuffer->size += tempNode->size;
//				_subBuffer->next = tempNode->next;
//
//				if (_subBuffer->next != nullptr)
//					_subBuffer->next->prev = _subBuffer;
//
//				delete tempNode;
//				--fragments;
//			}
//		}
//
//		if (_subBuffer->prev != nullptr)
//		{
//			if (_subBuffer->prev->inUse == false)
//			{// Join prev
//				MappedBuffer::Node* tempNode = _subBuffer->prev;
//				_subBuffer->size += tempNode->size;
//				_subBuffer->offset = tempNode->offset;
//				_subBuffer->prev = tempNode->prev;
//
//				if (_subBuffer->prev != nullptr)
//					_subBuffer->prev->next = _subBuffer;
//
//				delete tempNode;
//				--fragments;
//			}
//		}
//	}
//	void Clear()
//	{
//		MappedBuffer::Node* currentBuffer = subBufferMapHead;
//
//		for (;;)
//		{
//			if (subBufferMapHead == nullptr)
//				break;
//
//			currentBuffer = subBufferMapHead->next;
//
//			delete subBufferMapHead;
//
//			subBufferMapHead = currentBuffer;
//		}
//
//		subBufferMapTail = nullptr;
//		fragments = 0;
//		size = 0;
//
//		// Empty GPU buffer
//	}
//
//	void cout()
//	{
//		system("cls");
//		std::cout << "Size = " << this->size << "\nFragments = " << this->fragments << "\nHead = " << this->subBufferMapHead << "\nTail = " << this->subBufferMapTail << "\n\n";
//
//		MappedBuffer::Node* currentBuffer = subBufferMapHead;
//		uint64_t count = 0;
//		uint64_t sum = 0;
//		while (currentBuffer != nullptr)
//		{
//			sum += currentBuffer->size;
//
//			std::cout << "	Index = " << count << "\nCurrentBuffer = " << currentBuffer << "\nInUse = ";
//			if (currentBuffer->inUse == true)
//				std::cout << "true";
//			else
//				std::cout << "false";
//			std::cout << "\nOffset = " << currentBuffer->offset << "\nSize = " << currentBuffer->size << "\nPrev = " << currentBuffer->prev << "\nNext = " << currentBuffer->next << "\n\n";
//
//			currentBuffer = currentBuffer->next;
//
//			++count;
//		}
//
//		std::cout << "Count = " << count << "\nSum = " << sum;
//	}
//};


#if _DEBUG
int main()
#else
int WinMain(HINSTANCE _hInstance, HINSTANCE _hPrevInstance, LPSTR _lpCmdLine, int _nCmdShow)
#endif
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(-1);

	std::cout << "Controls: ARROWS\nQWEASD\nUIOJKL\nUIOJKL + CTRL\nUIOJKL + SHIFT\n";

	//VkU::MappedBuffer mappedBuffer;
	//mappedBuffer.Init(1024);
	//mappedBuffer.Cout();
	//
	//VkU::MappedBuffer::Node* a = mappedBuffer.AllocateSubBuffer(4);
	//mappedBuffer.Cout();
	//VkU::MappedBuffer::Node* b = mappedBuffer.AllocateSubBuffer(8);
	//mappedBuffer.Cout();
	//VkU::MappedBuffer::Node* c = mappedBuffer.AllocateSubBuffer(16);
	//mappedBuffer.Cout();
	//VkU::MappedBuffer::Node* d = mappedBuffer.AllocateSubBuffer(32);
	//mappedBuffer.Cout();
	//VkU::MappedBuffer::Node* e = mappedBuffer.AllocateSubBuffer(64);
	//mappedBuffer.Cout();
	//VkU::MappedBuffer::Node* null = mappedBuffer.AllocateSubBuffer(1025);
	//mappedBuffer.Cout();
	//
	//mappedBuffer.FreeSubBuffer(b);
	//mappedBuffer.Cout();
	//mappedBuffer.FreeSubBuffer(d);
	//mappedBuffer.Cout();
	//mappedBuffer.FreeSubBuffer(c);
	//mappedBuffer.Cout();
	//
	//VkU::MappedBuffer::Node* f = mappedBuffer.AllocateSubBuffer(56);
	//mappedBuffer.Cout();
	//
	//mappedBuffer.Clear();
	//mappedBuffer.Cout();
	//
	//return 0;

	Engine::Init();
	Engine::Loop();
	Engine::ShutDown();

	return 0;
}