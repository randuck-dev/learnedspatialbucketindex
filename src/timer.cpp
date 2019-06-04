#include "learnedindex/timer.h"

#include <chrono>
#include <iostream>
namespace utils {
	void Timer::start() {
		m_StartTime = std::chrono::system_clock::now();
		m_bRunning = true;
	}

	void Timer::stop(bool show) {
		m_EndTime = std::chrono::system_clock::now();
		m_bRunning = false;
		if (show)
			showElapsed();
	}

	double Timer::elapsedMilliseconds() {
		std::chrono::time_point<std::chrono::system_clock> endTime;

		if (m_bRunning) {
			endTime = std::chrono::system_clock::now();
		}
		else {
			endTime = m_EndTime;
		}

		return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_StartTime).count();
	}

	double Timer::elapsedSeconds() {
		return elapsedMilliseconds() / 1000.0;
	}

	double Timer::elapsedNanoSeconds() {
		return std::chrono::duration_cast<std::chrono::nanoseconds>(m_EndTime - m_StartTime).count();
	}

	void Timer::showElapsed() {
		std::cout << "Seconds: " << elapsedSeconds() << std::endl;
		std::cout << "Milliseconds: " << elapsedMilliseconds() << std::endl;
		std::cout << "Nanoseconds: " << elapsedNanoSeconds() << std::endl;
	}
}