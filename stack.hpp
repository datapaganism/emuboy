#pragma once
#include <array>
#include "config.hpp"


template <class T, int max_size>
class StackT
{
public:
	StackT();
	std::array<T, max_size> queue;
	int tail_pos = 0;
	int head_pos = 0;
	bool empty = true;
	bool full = false;

	void push(T elem);
	int elemCount();
	T pop();
	void popBy(int count);
	void reset();

};



template<class T, int max_size>
inline StackT<T, max_size>::StackT()
{
}

template<class T, int max_size>
inline void StackT<T, max_size>::push(T elem)
{
	if (!full)
	{
		queue[tail_pos++] = elem;
		if (tail_pos >= max_size)
			tail_pos = 0;

		empty = false;
		full = head_pos == tail_pos;
		return;
	}
	//fprintf(stderr, "cannot push full StackT<T, max_size>");  exit(-1);
}


template<class T, int max_size>
inline int StackT<T, max_size>::elemCount()
{
	if (head_pos > tail_pos)
		return (fifo_max_size - head_pos) + tail_pos + 1;
	return (tail_pos - head_pos) + 1;
}

template<class T, int max_size>
inline T StackT<T, max_size>::pop()
{
	if (!empty)
	{
		T* temp = &queue[head_pos++];
		if (head_pos >= max_size)
			head_pos = 0;

		full = false;
		empty = head_pos == tail_pos;
		return *temp;
	}
	//fprintf(stderr, "cannot pop empty StackT<T, max_size>");  exit(-1);
}

template<class T, int max_size>
inline void StackT<T, max_size>::popBy(int count)
{
	if (!empty)
	{
		head_pos += count;
		if (head_pos >= fifo_max_size)
			head_pos = 0;
		full = false;
		empty = head_pos == tail_pos;
		return;
	}
}

template<class T, int max_size>
inline void StackT<T, max_size>::reset()
{
	tail_pos = 0;
	head_pos = 0;
	empty = true;
	full = false;
}