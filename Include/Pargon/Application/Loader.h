#pragma once

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>

namespace Pargon
{
	class Loader
	{
	public:
		struct Task
		{
			virtual ~Task() = default;
			virtual void Execute() = 0;
		};

		Loader();
		~Loader();

		auto IsRunning() const -> bool;

		void AddTask(std::unique_ptr<Task>&& task, int priority);
		void CancelTasks();
		void WaitForCompletion();
		void Process();

	private:
		struct TaskEntry
		{
			mutable std::unique_ptr<Task> Task;
			int Priority;

			auto operator<(const TaskEntry& other) const -> bool { return Priority < other.Priority; }
		};

		std::priority_queue<TaskEntry> _tasks;

		std::thread _thread;
		std::mutex _mutex;
		std::condition_variable _condition;
	
		std::atomic<bool> _running;
		std::atomic<bool> _waiting;
		std::atomic<bool> _quit;

		void StartThread();
		void StopThread();
		void Thread();
	};
}
