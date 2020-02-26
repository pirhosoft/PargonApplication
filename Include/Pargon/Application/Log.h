#pragma once

#include "Pargon/Containers/Function.h"
#include "Pargon/Containers/List.h"
#include "Pargon/Containers/String.h"
#include "Pargon/Serialization/StringWriter.h"

#include <mutex>

namespace Pargon
{
	class File;

	class Log
	{
	public:
		using OutputFunction = Function<void(StringView, StringView)>;

		static void NameCurrentThread(StringView name);
		static auto CurrentThreadName() -> StringView;

		Log();

		void AddOutputFunction(OutputFunction&& function);

		template<typename... Ts> void Write(StringView format, const Ts&... inputs);
		void Write(StringView message);

	private:
		std::mutex _mutex;
		List<Function<void(StringView, StringView)>> _printers;
	};
}

template<typename... Ts>
void Pargon::Log::Write(StringView format, const Ts&... inputs)
{
	auto message = FormatString(format, inputs...);
	Write(message);
}
