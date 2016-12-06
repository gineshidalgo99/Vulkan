#include "Utilities.h"
#include <algorithm>
#include <thread>

double Utilities::sleepGraphicsThreadIfRequired(const double microsecondsPerFrame, std::chrono::high_resolution_clock::time_point & clock)
{
	const auto end = std::chrono::high_resolution_clock::now();
	const double inverseFpsUs = { std::chrono::duration_cast<std::chrono::nanoseconds>(end - clock).count() / 1e3 };

	clock = { std::chrono::high_resolution_clock::now() };

	if (microsecondsPerFrame > inverseFpsUs)
		std::this_thread::sleep_for(std::chrono::microseconds{ (int)std::round(std::max(microsecondsPerFrame / 2,
			microsecondsPerFrame - inverseFpsUs)) });

	return inverseFpsUs;
}
