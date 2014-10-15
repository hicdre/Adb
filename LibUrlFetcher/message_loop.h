#pragma once
#include <functional>
#include <deque>
#include "lock.h"

namespace base
{
	/*
	*	MessageLoop loop;
		while(continue_run_loop) {
			if (loop.WaitForTask(INFINITE))
				loop.RunTasks();
		};
	*/
	class MessageLoop
	{
	public:
		typedef std::function<void(void)> Task;
		MessageLoop();
		virtual ~MessageLoop();

		static MessageLoop* current();

		void Post(Task task);

		virtual void OnAddNewTask();
		virtual bool WaitForTasks(DWORD millseconds);
		void RunTasks();
	protected:
		Lock lock_;
		std::deque<Task> incoming_queue_;
		HANDLE event_has_work_;
	};
}


