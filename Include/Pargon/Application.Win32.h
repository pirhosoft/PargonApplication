#pragma once

#include "Pargon/Containers/Function.h"
#include "Pargon/Containers/List.h"
#include "Pargon/Containers/Map.h"
#include "Pargon/Containers/String.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace Pargon
{
	class ApplicationData
	{
	public:
		using WindowHandler = Function<bool(WPARAM, LPARAM)>;

		auto Window() const->HWND;
		void AddHandler(UINT message, WindowHandler&& handler);

	private:
		friend class Application;

		static LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM w, LPARAM l);

		HWND _window = nullptr;
		Map<UINT, WindowHandler> _handlers;
	};
}

inline
auto Pargon::ApplicationData::Window() const -> HWND
{
	return _window;
}

inline
void Pargon::ApplicationData::AddHandler(UINT message, WindowHandler&& handler)
{
	_handlers.AddOrSet(message, std::move(handler));
}
