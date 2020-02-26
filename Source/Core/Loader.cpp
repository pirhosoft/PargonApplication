#include "Pargon/Application/Loader.h"
#include "Pargon/Application/Log.h"

using namespace Pargon;

Loader::Loader()
{
	StartThread();
}

Loader::~Loader()
{
	StopThread();
}

auto Loader::IsRunning() const -> bool
{
	return _running || !_tasks.empty();
}

void Loader::AddTask(std::unique_ptr<Task>&& task, int priority)
{
	if (std::this_thread::get_id() == _thread.get_id())
	{
		task->Execute();
	}
	else
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_tasks.push({ std::move(task), priority });
	}
}

void Loader::CancelTasks()
{
	{
		std::lock_guard<std::mutex> lock(_mutex);

		while (!_tasks.empty())
			_tasks.pop();
	}
}

void Loader::WaitForCompletion()
{
	while (IsRunning())
		std::this_thread::yield();
}

void Loader::Process()
{
	if (_waiting && !_tasks.empty())
	{
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_waiting = false;
			_running = true;
		}

		_condition.notify_one();
	}
}

void Loader::StartThread()
{
	_running = false;
	_waiting = false;
	_quit = false;

	_thread = std::thread(&Loader::Thread, this);
}

void Loader::StopThread()
{
	{
		std::lock_guard<std::mutex> lock(_mutex);
		while (!_tasks.empty()) _tasks.pop();
		_waiting = false;
		_quit = true;
	}

	_condition.notify_one();
	_thread.join();
}

void Loader::Thread()
{
	Log::NameCurrentThread("Loader");

	while (!_quit)
	{
		std::unique_ptr<Task> task = nullptr;

		{
			_waiting = true;
			std::unique_lock<std::mutex> lock(_mutex);

			while (_waiting)
				_condition.wait(lock);

			if (!_tasks.empty())
			{
				task = std::move(_tasks.top().Task);
				_tasks.pop();
			}
		}

		if (task != nullptr)
			task->Execute();

		_running = false;
	}
}
