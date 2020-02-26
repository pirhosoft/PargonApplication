#include "Pargon/Application/Clipboard.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

using namespace Pargon;

void Clipboard::SetText(StringView text)
{
	if (OpenClipboard(NULL))
	{
		EmptyClipboard();

		auto buffer = GlobalAlloc(GMEM_MOVEABLE, text.Length() + 1);

		if (buffer != nullptr)
		{
			auto data = (char*)GlobalLock(buffer);

			if (data != nullptr)
			{
				std::copy(text.begin(), text.end(), data);
				GlobalUnlock(buffer);
			}

			SetClipboardData(CF_TEXT, buffer);
		}

		CloseClipboard();
	}
}

auto Clipboard::GetText() -> String
{
	auto text = ""_s;

	if (IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard(NULL))
	{
		auto buffer = GetClipboardData(CF_TEXT);
		auto data = (char*)GlobalLock(buffer);

		if (data != nullptr)
		{
			text = String(data, static_cast<int>(std::char_traits<char>::length(data)));
			GlobalUnlock(buffer);
		}

		CloseClipboard();
	}

	return text;
}
