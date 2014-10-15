#include "stdafx.h"
#include "message_loop.h"

#include <unordered_map>
#include "lock.h"

namespace base
{
	namespace
	{
		class MessageLoopOwner
		{
		public:
			~MessageLoopOwner()
			{
				for (auto iter : loops_)
				{
					delete iter.second;
				}
			}
			MessageLoop* Get()
			{
				DWORD thread_id = GetCurrentThreadId();
				AutoLock locker(lock_);
				if (loops_.count(thread_id))
					return loops_.at(thread_id);
				return NULL;
			}
			void Set(MessageLoop* v)
			{
				DWORD thread_id = GetCurrentThreadId();
				AutoLock locker(lock_);
				assert(!loops_.count(thread_id));
				loops_[thread_id] = v;
			}

			void Remove()
			{
				DWORD thread_id = GetCurrentThreadId();
				AutoLock locker(lock_);
				loops_.erase(thread_id);
			}
		private:
			std::unordered_map<DWORD, MessageLoop*> loops_;
			Lock lock_;
		};

		static MessageLoopOwner s_loop_owner;

	}
	
	MessageLoop* MessageLoop::current()
	{
		return s_loop_owner.Get();
	}

	void MessageLoop::Post(Task task)
	{
		{
			AutoLock locker(lock_);
			incoming_queue_.push_back(task);
		}

		OnAddNewTask();
	}


	void MessageLoop::OnAddNewTask()
	{
		::SetEvent(event_has_work_);
	}

	void MessageLoop::RunTasks()
	{
		std::deque<Task> work_queue_;
		{
			AutoLock locker(lock_);
			if (incoming_queue_.empty())
				return;
			incoming_queue_.swap(work_queue_);
		}

		while (!work_queue_.empty())
		{
			work_queue_.front()();
			work_queue_.pop_front();
		}
	}


	bool MessageLoop::WaitForTasks(DWORD millseconds)
	{
		
		if (WAIT_OBJECT_0 == WaitForSingleObject(event_has_work_, millseconds))
			return true;
		return false;
	}

	MessageLoop::MessageLoop()
	{
		event_has_work_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		s_loop_owner.Set(this);
	}

	MessageLoop::~MessageLoop()
	{
		CloseHandle(event_has_work_);
		s_loop_owner.Remove();
	}
}