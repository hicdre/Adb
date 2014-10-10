#pragma once
#include "basictypes.h"

namespace base
{
	class Lock
	{
	public:
		Lock();
		~Lock();

		bool Try();

		// Take the lock, blocking until it is available if necessary.
		void Dolock();

		// Release the lock.  This must only be called by the lock's holder: after
		// a successful call to Try, or a call to Lock.
		void Unlock();

	private:
		CRITICAL_SECTION cs;
		DISALLOW_COPY_AND_ASSIGN(Lock);
	};

	class AutoLock
	{
	public:
		explicit AutoLock(Lock& m);
		~AutoLock();

	private:
		Lock& m_;
		DISALLOW_COPY_AND_ASSIGN(AutoLock);
	};
}