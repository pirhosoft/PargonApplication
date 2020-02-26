#pragma once

#include "Pargon/Containers/List.h"
#include "Pargon/Containers/String.h"
#include "Pargon/Serialization/Serialization.h"
#include "Pargon/Types/Time.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

namespace Pargon
{
	class Log;
	class ApplicationData;

	enum class ApplicationState
	{
		Startup,
		Background,
		Foreground,
		Focused,
		Shutdown
	};

	template<> auto EnumNames<ApplicationState> = SetEnumNames
	(
		"Startup",
		"Background",
		"Foreground",
		"Focused",
		"Shutdown"
	);

	enum class WindowStyle
	{
		Frameless,
		Static,
		Resizable
	};

	template<> auto EnumNames<WindowStyle> = SetEnumNames
	(
		"Frameless",
		"Static",
		"Resizable"
	);

	enum class InputType
	{
		Native,
		Emulated,
		Unsupported
	};

	template<> auto EnumNames<InputType> = SetEnumNames
	(
		"Native",
		"Emulated",
		"Unsupported"
	);

	struct ApplicationInformation
	{
		String Name;
		String Platform;
		String Device;
		String Version;
		String Language;
		List<String> Options;
		bool IsDebug;

		unsigned int ScreenWidth;
		unsigned int ScreenHeight;

		List<WindowStyle> SupportedWindowStyles;
		InputType KeyboardType;
		InputType MouseType;
		InputType ControllerType;
		InputType TouchType;
		InputType MotionType;
		List<String> AvailableRenderers;

		void WriteInformation(Log& log);
		void WriteCapabilities(Log& log);
	};

	class ApplicationInterface
	{
	protected:
		friend class Application;

		virtual void Setup() {}
		virtual void Start() {}
		virtual void Activate() {}
		virtual void Process(Time elapsed) {}
		virtual void Deactivate() {}
		virtual void Stop() {}
		virtual void Shutdown() {}
	};

	struct WindowSettings
	{
		String Title;
		WindowStyle Style;

		unsigned int X;
		unsigned int Y;
		unsigned int Width;
		unsigned int Height;
	};

	class Application
	{
	public:
		Application();
		~Application();

		auto Interface() const -> ApplicationInterface*;
		auto Data() const -> ApplicationData*;
		auto State() const -> ApplicationState;

		auto Initialize(StringView name, ApplicationInterface& interface) -> ApplicationInformation;
		void Run(const WindowSettings& window);
		void Terminate();

	private:
		friend class ApplicationData;

		enum class StateChange
		{
			None,
			Start,
			Show,
			Focus,
			Blur,
			Hide
		};

		ApplicationInterface* _interface;
		std::unique_ptr<ApplicationData> _data;
		ApplicationState _state;
		Time _processTime;

		std::thread _applicationThread;
		std::thread::id _systemThreadId;
		std::thread::id _applicationThreadId;
		std::atomic<bool> _applicationRunning;

		std::atomic<StateChange> _stateChange;
		std::mutex _stateChangeMutex;
		std::condition_variable _stateChangeSignal;

		void Initialize(ApplicationInformation& information);
		void SetupWindow(const WindowSettings& window);
		void RunSystemThread();
		void RunApplicationThread();
		void TerminateSystemThread();

		void ChangeState(StateChange state);
		void Process();
	};
}

inline
auto Pargon::Application::Interface() const -> ApplicationInterface*
{
	return _interface;
}

inline
auto Pargon::Application::Data() const -> ApplicationData*
{
	return _data.get();
}

inline
auto Pargon::Application::State() const -> ApplicationState
{
	return _state;
}
