#pragma once

#include "Pargon/Containers/String.h"

namespace Pargon
{
	class Clipboard
	{
	public:
		static void SetText(StringView text);
		static auto GetText() -> String;
	};
}
