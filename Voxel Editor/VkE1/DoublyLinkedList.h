#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H

template <class T>
class DoublyLinkedList
{
public:
	struct Node
	{
		T data;
		Node* prev;
		Node* next;

		static inline Node GetNode(T _data, Node* _prev, Node* _next)
		{
			return { _data, _prev, _next };
		}
	};

	size_t size = 0;
	Node* head = nullptr;
	Node* tail = nullptr;

	void Push_Head(T _data);
	void Push_Tail(T _data);
	void PushBefore(Node* _node, T _data);
	void PushAfter(Node* _node, T _data);

	Node* GetAt(size_t _index);

	void Delete(Node* _node);
	void DeleteAt(size_t _index);

	void Clear();
};

template <class T>
void DoublyLinkedList<T>::Push_Head(T _data)
{
	if (head != nullptr)
	{
		Node* newHead = new Node;
		(*newHead) = Node::GetNode(_data, nullptr, head);
		head->prev = newHead;
		head = newHead;
	}
	else
	{
		head = new Node;
		tail = head;
		(*head) = Node::GetNode(_data, nullptr, nullptr);
	}

	++size;
}
template <class T>
void DoublyLinkedList<T>::Push_Tail(T _data)
{
	if (tail != nullptr)
	{
		Node* newTail = new Node;
		(*newTail) = Node::GetNode(_data, tail, nullptr);
		tail->next = newTail;
		tail = newTail;
	}
	else
	{
		tail = new Node;
		head = tail;
		(*tail) = Node::GetNode(_data, nullptr, nullptr);
	}

	++size;
}
template <class T>
void DoublyLinkedList<T>::PushBefore(Node* _node, T _data)
{
	if (_node != nullptr)
	{
		Node* newNode = new Node;
		newNode = Node::GetNode(_node->prev, _node, _data);

		if (newNode->prev != nullptr)
			newNode->prev->next = newNode;

		_node->prev = newNode;
	}
	else
	{
		head = new Node;
		tail = head;
		(*head) = Node::GetNode(_data, nullptr, nullptr);
	}

	++size;
}
template <class T>
void DoublyLinkedList<T>::PushAfter(Node* _node, T _data)
{
	if (_node != nullptr)
	{
		Node* newNode = new Node;
		newNode = Node::GetNode(_node, _node->next, _data);

		if (newNode->next != nullptr)
			newNode->next->prev = newNode;

		_node->next = newNode;
	}
	else
	{
		head = new Node;
		tail = head;
		(*head) = Node::GetNode(_data, nullptr, nullptr);
	}

	++size;
}

template <class T>
typename DoublyLinkedList<T>::Node* DoublyLinkedList<T>::GetAt(size_t _index)
{
	if(_index >= size)
		return nullptr;

	Node* curr = head;
	for (size_t i = 0; i != _index; ++i)
		curr = curr->next;

	return curr;
}

template <class T>
void DoublyLinkedList<T>::Delete(Node* _node)
{

}
template <class T>
void DoublyLinkedList<T>::DeleteAt(size_t _index)
{

}

template <class T>
void DoublyLinkedList<T>::Clear()
{
	Node* next = head->next;
	while (head != nullptr)
	{
		next = head->next;
		delete head;
		head = next;
	}

	head = nullptr;
	tail = nullptr;
	size = 0;
}

#endif