#include "Pargon/Application/Log.h"

using namespace Pargon;

namespace
{
	thread_local String _threadName = "Unknown"_s;
}

void Log::NameCurrentThread(StringView name)
{
	_threadName = name;
}

auto Log::CurrentThreadName() -> StringView
{
	return _threadName;
}

void Log::AddOutputFunction(OutputFunction&& function)
{
	_printers.Add(std::move(function));
}

void Log::Write(StringView message)
{
	for (auto& printer : _printers)
		printer(_threadName, message);
}
