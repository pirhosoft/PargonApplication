#include "Pargon/Application/Log.h"

#include <iostream>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

using namespace Pargon;

namespace
{
	void DebuggerOutput(StringView thread, StringView message)
	{
		auto text = Text(message.GetString());
		auto wide = text.AsWide();

		OutputDebugStringA(thread.begin());
		OutputDebugStringA(" >> ");
		OutputDebugString(wide.data());
		OutputDebugStringA("\n");
	}

	void ConsoleOutput(StringView thread, StringView message)
	{
		std::cout.write(thread.begin(), thread.Length());
		std::cout.write(" >> ", 4);
		std::cout.write(message.begin(), message.Length());
		std::cout.write("\n", 1);
	}
}

Log::Log()
{
	if (IsDebuggerPresent())
		AddOutputFunction(DebuggerOutput);
	else
		AddOutputFunction(ConsoleOutput);
}
