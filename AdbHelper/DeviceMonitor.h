#pragma once

namespace adb
{
	class DeviceMonitor
	{
	public:
		class Observer
		{
		public:
			virtual ~Observer() {}
			virtual void OnDeviceStateChanged() {}
		};
		DeviceMonitor();


		void StartMonitor();

		void StopMonitor();


	private:
		HANDLE monitor_thread_;
		HANDLE event_stop_;

	};
}