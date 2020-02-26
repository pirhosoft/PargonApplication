#pragma once

#include "Pargon/Containers/Array.h"

namespace Pargon
{
	template<typename EventType, int N>
	class EventStream
	{
	public:
		auto NextIndex(int currentIndex) const -> int;
		auto CurrentIndex() const -> int;

		auto Event(int index) const -> const EventType&;
		void AddEvent(const EventType& item);

	private:
		Array<EventType, N> _array;

		int _currentIndex = 0;
	};
}

template<typename EventType, int N>
auto Pargon::EventStream<EventType, N>::NextIndex(int currentIndex) const -> int
{
	return currentIndex >= N - 1 ? 0 : currentIndex + 1;
}

template<typename EventType, int N>
auto Pargon::EventStream<EventType, N>::CurrentIndex() const -> int
{
	return _currentIndex;
}

template<typename EventType, int N>
auto Pargon::EventStream<EventType, N>::Event(int index) const -> const EventType&
{
	return _array.Item(index);
}

template<typename EventType, int N>
void Pargon::EventStream<EventType, N>::AddEvent(const EventType& value)
{
	auto nextIndex = _currentIndex >= N - 1 ? 0 : _currentIndex + 1;
	_array.SetItem(_currentIndex, value);
	_currentIndex = nextIndex;
}
