#pragma once

#include <chrono>

namespace utils {
	class Timer {
	public:
		void start();

		void stop(bool show = true);

		double elapsedMilliseconds();

		double elapsedSeconds();

		double elapsedNanoSeconds();

		void showElapsed();

	private:
		std::chrono::time_point<std::chrono::system_clock> m_StartTime;
		std::chrono::time_point<std::chrono::system_clock> m_EndTime;
		bool m_bRunning = false;
	};
}