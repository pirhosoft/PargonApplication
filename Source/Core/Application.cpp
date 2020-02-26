#include "Pargon/Application/Application.h"
#include "Pargon/Application/Log.h"

#include <cassert>

using namespace Pargon;

void ApplicationInformation::WriteInformation(Log& log)
{
	auto information = R"(Application Information
 - Name: {}
 - Platform: {}
 - Device: {}
 - Version: {}
 - Language: {}
 - Options: {|-}
 - IsDebug: {})"_sv;
	
	log.Write(information, Name, Platform, Device, Version, Language, Options, IsDebug);
}

void ApplicationInformation::WriteCapabilities(Log& log)
{
	auto capabilities = R"(Application Capabilities
 - Screen Size: {}x{}
 - Window Styles: {|-}
 - Keyboard: {}
 - Mouse: {}
 - Controller: {}
 - Touch: {}
 - Motion: {}
 - Renderers: {|-})"_sv;

	log.Write(capabilities, ScreenWidth, ScreenHeight, SupportedWindowStyles, KeyboardType, MouseType, ControllerType, TouchType, MotionType, AvailableRenderers);
}

namespace
{
	void SetState(ApplicationState& state, ApplicationState from, ApplicationState to)
	{
		assert(state == from);
		state = to;
	}
}

auto Application::Initialize(StringView name, ApplicationInterface& interface) -> ApplicationInformation
{
	Log::NameCurrentThread("System");

	_interface = std::addressof(interface);
	_state = ApplicationState::Startup;

	ApplicationInformation information;
	information.Name = name;

	Initialize(information);

	return information;
}

void Application::Run(const WindowSettings& window)
{
	SetupWindow(window);

	_systemThreadId = std::this_thread::get_id();
	_stateChange.store(StateChange::None);
	_applicationRunning.store(true);

	_applicationThread = std::thread([](Application* application)
	{
		Log::NameCurrentThread("Application");

		application->_applicationThreadId = std::this_thread::get_id();
		application->_interface->Setup();
		application->_processTime = GetCurrentTime();
		application->RunApplicationThread();

		SetState(application->_state, ApplicationState::Background, ApplicationState::Shutdown);

		application->_interface->Shutdown();
	}, this);

	RunSystemThread();
}

void Application::Terminate()
{
	if (std::this_thread::get_id() != _applicationThreadId)
	{
		_applicationRunning.store(false);
		_applicationThread.join();
	}
	else
	{
		TerminateSystemThread();
	}
}

void Application::ChangeState(StateChange state)
{
	if (std::this_thread::get_id() != _applicationThreadId)
	{
		_stateChange.store(state);
		std::unique_lock<std::mutex> lock(_stateChangeMutex);
		_stateChangeSignal.wait(lock, [&]{ return _stateChange == StateChange::None; });
	}
	else
	{
		switch (state)
		{
			case StateChange::Start: SetState(_state, ApplicationState::Startup, ApplicationState::Foreground); _interface->Start(); break;
			case StateChange::Show: SetState(_state, ApplicationState::Background, ApplicationState::Foreground); _interface->Start(); break;
			case StateChange::Focus: SetState(_state, ApplicationState::Foreground, ApplicationState::Focused); _interface->Activate(); break;
			case StateChange::Blur: SetState(_state, ApplicationState::Focused, ApplicationState::Foreground); _interface->Deactivate(); break;
			case StateChange::Hide: SetState(_state, ApplicationState::Foreground, ApplicationState::Background); _interface->Stop(); break;
			case StateChange::None: break;
		}

		_stateChange.store(StateChange::None);
		_stateChangeSignal.notify_all();
	}
}

void Application::Process()
{
	assert(std::this_thread::get_id() == _applicationThread.get_id());

	if (_stateChange != StateChange::None)
		ChangeState(_stateChange);

	auto time = GetCurrentTime();

	_interface->Process(time - _processTime);
	_processTime = time;
}
