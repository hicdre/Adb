#include "stdafx.h"
#include "lock.h"

namespace base
{
	Lock::Lock()
	{
		::InitializeCriticalSectionAndSpinCount(&cs, 2000);
	}

	Lock::~Lock()
	{
		::DeleteCriticalSection(&cs);
	}

	bool Lock::Try()
	{
		if (::TryEnterCriticalSection(&cs) != FALSE) {
			return true;
		}
		return false;
	}

	void Lock::Dolock()
	{
		::EnterCriticalSection(&cs);
	}

	void Lock::Unlock()
	{
		::LeaveCriticalSection(&cs);
	}

	AutoLock::AutoLock(Lock& m)
		: m_(m)
	{
		m_.Dolock();
	}

	AutoLock::~AutoLock()
	{
		m_.Unlock();
	}
}