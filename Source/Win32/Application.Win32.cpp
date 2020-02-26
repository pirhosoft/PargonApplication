#include "Pargon/Application/Application.h"
#include "Pargon/Application/Log.h"
#include "Pargon/Application.Win32.h"

#include <cassert>
#include <codecvt>
#include <shellapi.h>
#include <VersionHelpers.h>

using namespace Pargon;

Application::Application()
{
	_data = std::make_unique<ApplicationData>();
}

Application::~Application()
{
}

namespace
{
	void SetApplication(HWND window, LPARAM create)
	{
		auto application = reinterpret_cast<LPCREATESTRUCT>(create)->lpCreateParams;
		SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(application));
	}

	auto GetApplication(HWND window) -> Application*
	{
		auto application = GetWindowLongPtr(window, GWLP_USERDATA);
		return reinterpret_cast<Application*>(application);
	}

	auto GetWindowClass(HINSTANCE instance, WNDPROC windowProcedure) -> const wchar_t*
	{
		static bool _created = false;
		static WNDCLASSEX _class;

		auto name = L"PargonApplicationWindow";

		if (!_created)
		{
			_class.cbSize = sizeof(WNDCLASSEX);
			_class.style = CS_HREDRAW | CS_VREDRAW;
			_class.lpfnWndProc = windowProcedure;
			_class.cbClsExtra = 0;
			_class.cbWndExtra = 0;
			_class.hInstance = instance;
			_class.hIcon = LoadIconA(instance, MAKEINTRESOURCEA(101));
			_class.hCursor = LoadCursor(NULL, IDC_ARROW);
			_class.hbrBackground = (HBRUSH)COLOR_WINDOW;
			_class.lpszMenuName = NULL;
			_class.lpszClassName = name;
			_class.hIconSm = NULL;

			auto atom = RegisterClassEx(&_class);
			assert(atom != NULL);
		}

		return name;
	}

	auto GetTitle(StringView name) -> std::wstring
	{
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(name.begin(), name.end());
	}

	auto GetWindowStyle(const WindowSettings& settings) -> DWORD
	{
		switch (settings.Style)
		{
			case WindowStyle::Frameless: return WS_POPUP;
			case WindowStyle::Static: return WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
			case WindowStyle::Resizable: return WS_OVERLAPPEDWINDOW;
		}

		return 0;
	}

	auto GetWindowBounds(const WindowSettings& settings, DWORD style) -> RECT
	{
		RECT bounds;

		auto desktop = GetDesktopWindow();
		GetWindowRect(desktop, &bounds);

		bounds.left = static_cast<LONG>(settings.X);
		bounds.top = static_cast<LONG>(settings.Y);
		bounds.right = static_cast<LONG>(settings.X + settings.Width);
		bounds.bottom = static_cast<LONG>(settings.Y + settings.Height);

		AdjustWindowRect(&bounds, style, false);
		return bounds;
	}

	auto MakeWindow(const WindowSettings& settings, Application* application, WNDPROC windowProcedure) -> HWND
	{
		auto instance = GetModuleHandle(0);
		auto className = GetWindowClass(instance, windowProcedure);
		auto title = GetTitle(settings.Title);
		auto style = GetWindowStyle(settings);
		auto bounds = GetWindowBounds(settings, style);

		return CreateWindowEx(NULL, className, title.c_str(), style, bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top, NULL, NULL, instance, application);
	}
}

LRESULT CALLBACK ApplicationData::WindowProcedure(HWND window, UINT message, WPARAM w, LPARAM l)
{
	auto application = GetApplication(window);

	if (application != nullptr)
	{
		auto index = application->_data->_handlers.GetIndex(message);
		if (index != Sequence::InvalidIndex)
		{
			auto handled = application->_data->_handlers.ItemAtIndex(index)(w, l);
			if (handled)
				return 0;
		}
	}

	switch (message)
	{
		case WM_CREATE:
		{
			SetApplication(window, l);
			break;
		}

		case WM_DESTROY:
		{
			application->_data->_window = NULL;
			PostQuitMessage(0);
			break;
		}

		case WM_USER:
		{
			DestroyWindow(window);
			return 0;
		}

		case WM_SYSCOMMAND:
		{
			if (w == SC_MINIMIZE)
			{
				if (application->State() == ApplicationState::Focused)
					application->ChangeState(Application::StateChange::Blur);

				application->ChangeState(Application::StateChange::Hide);
			}
			else if (w == SC_RESTORE)
			{
				if (application->State() != ApplicationState::Focused)
					application->ChangeState(Application::StateChange::Show);
			}

			break;
		}

		case WM_SETFOCUS:
		{
			if (application->State() == ApplicationState::Foreground)
				application->ChangeState(Application::StateChange::Focus);

			break;
		}

		case WM_KILLFOCUS:
		{
			if (application->State() == ApplicationState::Focused)
				application->ChangeState(Application::StateChange::Blur);

			break;
		}
	}

	return DefWindowProc(window, message, w, l);
}

namespace
{
	auto GetIsDebug() -> bool
	{
#ifdef _DEBUG
		return true;
#else
		return false;
#endif
	}

	auto GetDevice() -> String
	{
		return "PC"_s;
	}

	auto GetPlatform() -> String
	{
		return "Win32"_s;
	}

	auto GetWindowsVersion() -> String
	{
		if (IsWindows10OrGreater()) return "10"_s;
		else if (IsWindows8Point1OrGreater()) return "8.1"_s;
		else if (IsWindows8OrGreater()) return "8"_s;
		else if (IsWindows7SP1OrGreater()) return "7.1"_s;
		else if (IsWindows7OrGreater()) return "7"_s;

		return "Old"_s;
	}

	auto GetLanguage() -> String
	{
		ULONG count;
		ULONG size = 0;

		if (!GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &count, nullptr, &size) || count == 0)
			return "en-US"_s;

		auto buffer = std::make_unique<WCHAR[]>(size);
		GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &count, buffer.get(), &size);

		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(buffer.get());
	}

	auto GetOptions() -> List<String>
	{
		List<String> options;

		auto argc = 0;
		auto args = GetCommandLine();
		auto argv = CommandLineToArgvW(args, &argc);

		for (auto i = 1; i < argc; i++)
		{
			auto utf8 = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(argv[i]);
			options.Add(std::move(utf8));
		}

		LocalFree(argv);
		return options;
	}

	auto GetScreenWidth() -> unsigned int
	{
		return static_cast<unsigned int>(GetSystemMetrics(SM_CXSCREEN));
	}

	auto GetScreenHeight() -> unsigned int
	{
		return static_cast<unsigned int>(GetSystemMetrics(SM_CYSCREEN));
	}
}

void Application::Initialize(ApplicationInformation& information)
{
	information.Platform = GetPlatform();
	information.Device = GetDevice();
	information.Version = GetWindowsVersion();
	information.Language = GetLanguage();
	information.Options = GetOptions();
	information.IsDebug = GetIsDebug();
	information.ScreenWidth = GetScreenWidth();
	information.ScreenHeight = GetScreenHeight();

	information.SupportedWindowStyles;
	information.KeyboardType = InputType::Native;
	information.MouseType = InputType::Native;
	information.ControllerType = InputType::Native;
	information.TouchType = InputType::Emulated;
	information.MotionType = InputType::Emulated;
	//information.AvailableRenderers = ApplicationData::GetAvailableRenderers();
}

void Application::SetupWindow(const WindowSettings& window)
{
	_data->_window = MakeWindow(window, this, ApplicationData::WindowProcedure);
}

void Application::RunSystemThread()
{
	ShowWindow(_data->_window, SW_SHOW);

	MSG message;
	while (GetMessage(&message, NULL, 0, 0) != 0)
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	Terminate();
}

void Application::TerminateSystemThread()
{
	PostMessage(_data->_window, WM_USER, 0, 0);
}

void Application::RunApplicationThread()
{
	ChangeState(StateChange::Start);
	ChangeState(StateChange::Focus);

	while (_applicationRunning.load())
		Process();

	ChangeState(StateChange::Hide);
}
